#!/usr/bin/env python3
"""Submit an uploaded Xcode Cloud build for TestFlight beta review.

The script is intentionally stdlib-only: Xcode Cloud already provides Python and
OpenSSL, while the App Store Connect API key is supplied as a secret workflow
variable rather than committed to the repository.
"""

from __future__ import annotations

import base64
import json
import os
import subprocess
import tempfile
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path
from typing import Any


API_BASE = "https://api.appstoreconnect.apple.com/v1"
DEFAULT_APP_ID = "1543684531"  # QZ Fitness; public App Store ID.
POLL_SECONDS = 15
MAX_BUILD_POLLS = 40
MAX_STATE_POLLS = 20


def log(message: str) -> None:
    print(f"[beta-review] {message}", flush=True)


def required(name: str, *aliases: str) -> str:
    for candidate in (name, *aliases):
        value = os.environ.get(candidate, "").strip()
        if value:
            return value
    names = ", ".join((name, *aliases))
    raise RuntimeError(f"Required Xcode Cloud variable is missing: {names}")


def b64url(value: bytes) -> str:
    return base64.urlsafe_b64encode(value).rstrip(b"=").decode("ascii")


def der_to_jose(signature: bytes) -> bytes:
    """Convert OpenSSL's DER ECDSA signature into ES256 R||S form."""
    if len(signature) < 8 or signature[0] != 0x30:
        raise RuntimeError("OpenSSL returned an invalid ECDSA signature")

    index = 1
    length = signature[index]
    index += 1
    if length & 0x80:
        count = length & 0x7F
        length = int.from_bytes(signature[index : index + count], "big")
        index += count
    if length != len(signature) - index:
        raise RuntimeError("ECDSA DER signature length is inconsistent")

    if signature[index] != 0x02:
        raise RuntimeError("ECDSA DER signature has no R integer")
    r_length = signature[index + 1]
    r_start = index + 2
    r = signature[r_start : r_start + r_length]
    index = r_start + r_length

    if index + 2 > len(signature) or signature[index] != 0x02:
        raise RuntimeError("ECDSA DER signature has no S integer")
    s_length = signature[index + 1]
    s = signature[index + 2 : index + 2 + s_length]

    r = r.lstrip(b"\x00") or b"\x00"
    s = s.lstrip(b"\x00") or b"\x00"
    if len(r) > 32 or len(s) > 32:
        raise RuntimeError("ECDSA component is longer than ES256 allows")
    return r.rjust(32, b"\x00") + s.rjust(32, b"\x00")


class AppStoreConnect:
    def __init__(self) -> None:
        self.issuer = required(
            "ASC_ISSUER_ID",
            "APP_STORE_CONNECT_API_ISSUER_ID",
            "APP_STORE_CONNECT_ISSUER_ID",
        )
        self.key_id = required(
            "ASC_KEY_ID",
            "APP_STORE_CONNECT_API_KEY_ID",
            "APP_STORE_CONNECT_KEY_ID",
        )
        encoded_key = required(
            "ASC_KEY_P8_BASE64",
            "APP_STORE_CONNECT_API_KEY_P8_BASE64",
        )
        try:
            key_bytes = base64.b64decode(encoded_key, validate=True)
        except Exception as exc:  # pragma: no cover - provider input
            raise RuntimeError("ASC_KEY_P8_BASE64 is not valid base64") from exc
        if b"PRIVATE KEY-----" not in key_bytes:
            raise RuntimeError("ASC_KEY_P8_BASE64 does not contain a PEM private key")

        handle = tempfile.NamedTemporaryFile(prefix="asc-key-", suffix=".p8", delete=False)
        self.key_path = Path(handle.name)
        handle.write(key_bytes)
        handle.close()
        os.chmod(self.key_path, 0o600)

    def close(self) -> None:
        try:
            self.key_path.unlink()
        except FileNotFoundError:
            pass

    def jwt(self) -> str:
        now = int(time.time())
        header = {"alg": "ES256", "kid": self.key_id, "typ": "JWT"}
        payload = {
            "iss": self.issuer,
            "iat": now,
            "exp": now + 600,
            "aud": "appstoreconnect-v1",
        }
        unsigned = ".".join(
            (
                b64url(json.dumps(header, separators=(",", ":")).encode()),
                b64url(json.dumps(payload, separators=(",", ":")).encode()),
            )
        ).encode("ascii")
        result = subprocess.run(
            ["openssl", "dgst", "-sha256", "-sign", str(self.key_path)],
            input=unsigned,
            capture_output=True,
            check=False,
        )
        if result.returncode != 0:
            raise RuntimeError("OpenSSL could not sign the App Store Connect JWT")
        return unsigned.decode("ascii") + "." + b64url(der_to_jose(result.stdout))

    def request(self, method: str, path: str, payload: dict[str, Any] | None = None) -> tuple[int, dict[str, Any]]:
        body = None
        headers = {
            "Authorization": f"Bearer {self.jwt()}",
            "Accept": "application/json",
        }
        if payload is not None:
            body = json.dumps(payload).encode("utf-8")
            headers["Content-Type"] = "application/json"

        request = urllib.request.Request(
            API_BASE + path,
            data=body,
            headers=headers,
            method=method,
        )
        try:
            with urllib.request.urlopen(request, timeout=60) as response:
                return response.status, json.loads(response.read().decode("utf-8"))
        except urllib.error.HTTPError as error:
            raw = error.read().decode("utf-8", errors="replace")
            try:
                parsed = json.loads(raw)
            except json.JSONDecodeError:
                parsed = {"error": raw[:500]}
            return error.code, parsed
        except urllib.error.URLError as error:
            raise RuntimeError(f"App Store Connect request failed: {error.reason}") from error


def first_data(response: dict[str, Any]) -> dict[str, Any] | None:
    data = response.get("data")
    if isinstance(data, list):
        return data[0] if data else None
    return data if isinstance(data, dict) else None


def api_error(status: int, response: dict[str, Any]) -> str:
    errors = response.get("errors")
    if isinstance(errors, list) and errors:
        error = errors[0]
        return f"HTTP {status}: {error.get('code', 'API_ERROR')} - {error.get('detail', error.get('title', 'unknown error'))}"
    return f"HTTP {status}: App Store Connect request was rejected"


def find_build(api: AppStoreConnect, app_id: str, build_number: str) -> dict[str, Any]:
    query = urllib.parse.urlencode(
        {"filter[app]": app_id, "filter[version]": build_number, "limit": "50"}
    )
    for attempt in range(MAX_BUILD_POLLS):
        status, response = api.request("GET", f"/builds?{query}")
        if status != 200:
            raise RuntimeError(api_error(status, response))
        builds = response.get("data", [])
        if len(builds) > 1:
            raise RuntimeError(
                f"More than one App Store Connect build matches version {build_number}; refusing to guess"
            )
        if builds:
            build = builds[0]
            state = build.get("attributes", {}).get("processingState")
            log(f"Found App Store Connect build {build.get('id')} (processingState={state})")
            if state == "VALID":
                return build
            if state in {"FAILED", "INVALID"}:
                raise RuntimeError(f"App Store Connect build processing failed: {state}")
        if attempt + 1 < MAX_BUILD_POLLS:
            log(f"Waiting for build {build_number} to become visible/valid ({attempt + 1}/{MAX_BUILD_POLLS})")
            time.sleep(POLL_SECONDS)
    raise RuntimeError(f"App Store Connect build {build_number} did not become valid in time")


def ensure_compliance(api: AppStoreConnect, build: dict[str, Any]) -> dict[str, Any]:
    attributes = build.get("attributes", {})
    if attributes.get("usesNonExemptEncryption") is not None:
        return build
    build_id = build["id"]
    log("Export compliance is unset; declaring no non-exempt encryption")
    status, response = api.request(
        "PATCH",
        f"/builds/{build_id}",
        {
            "data": {
                "type": "builds",
                "id": build_id,
                "attributes": {"usesNonExemptEncryption": False},
            }
        },
    )
    if status != 200:
        raise RuntimeError(api_error(status, response))
    updated = first_data(response)
    if not updated or updated.get("attributes", {}).get("usesNonExemptEncryption") is not False:
        raise RuntimeError("Export-compliance update was not confirmed")
    return updated


def ensure_beta_ready(api: AppStoreConnect, build_id: str) -> None:
    for attempt in range(MAX_STATE_POLLS):
        status, response = api.request("GET", f"/builds/{build_id}/buildBetaDetail")
        if status != 200:
            raise RuntimeError(api_error(status, response))
        attributes = (first_data(response) or {}).get("attributes", {})
        external_state = attributes.get("externalBuildState")
        log(f"Beta state: internal={attributes.get('internalBuildState')} external={external_state}")
        if external_state not in {"MISSING_EXPORT_COMPLIANCE", "PROCESSING"}:
            return
        if attempt + 1 < MAX_STATE_POLLS:
            time.sleep(POLL_SECONDS)
    raise RuntimeError("Build did not leave the export-compliance/processing state in time")


def submit_beta_review(api: AppStoreConnect, build_id: str) -> None:
    status, response = api.request("GET", f"/builds/{build_id}/betaAppReviewSubmission")
    if status != 200:
        raise RuntimeError(api_error(status, response))
    existing = first_data(response)
    if existing:
        state = existing.get("attributes", {}).get("betaReviewState")
        log(f"Beta review submission already exists: {state}; no duplicate submission")
        if state in {"REJECTED", "DEVELOPER_REJECTED"}:
            raise RuntimeError(f"Existing beta review submission is in state {state}")
        return

    status, response = api.request(
        "POST",
        "/betaAppReviewSubmissions",
        {
            "data": {
                "type": "betaAppReviewSubmissions",
                "relationships": {
                    "build": {"data": {"type": "builds", "id": build_id}}
                },
            }
        },
    )
    if status not in {200, 201}:
        raise RuntimeError(api_error(status, response))
    created = first_data(response)
    if not created or created.get("id") != build_id:
        raise RuntimeError("Beta review submission response did not reference the target build")
    log(f"Beta review submitted: {created.get('attributes', {}).get('betaReviewState')}")


def main() -> int:
    if os.environ.get("CI_XCODEBUILD_ACTION", "").lower() != "archive":
        log("Not an archive action; skipping")
        return 0
    if os.environ.get("CI_XCODEBUILD_EXIT_CODE", "0") != "0":
        log("xcodebuild failed; skipping beta review submission")
        return 0

    build_number = required("CI_BUILD_NUMBER")
    app_id = os.environ.get("ASC_APP_ID", DEFAULT_APP_ID).strip()
    api = AppStoreConnect()
    try:
        build = find_build(api, app_id, build_number)
        build = ensure_compliance(api, build)
        ensure_beta_ready(api, build["id"])
        submit_beta_review(api, build["id"])
        return 0
    finally:
        api.close()


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        log(f"ERROR: {error}")
        raise SystemExit(1)
