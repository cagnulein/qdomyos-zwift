#!/usr/bin/env python3
"""Prints the next Android versionCode to use for a Play Store upload of
qdomyos-zwift, computed from the app's actual current state on Play Console
(max versionCode across all tracks, excluding Wear OS tracks which use a
separate, much higher numbering range) rather than from any locally-stored
counter. Read-only: opens a draft edit, reads track data, then deletes the
edit without ever committing it - zero effect on the live app.

Requires only the Python stdlib plus the `openssl` binary (used to sign the
service-account JWT), so it needs no extra pip install step in CI.
"""
import base64
import json
import subprocess
import sys
import tempfile
import time
import urllib.request
import urllib.error

SCOPE = "https://www.googleapis.com/auth/androidpublisher"


def b64url(data: bytes) -> str:
    return base64.urlsafe_b64encode(data).rstrip(b"=").decode()


def get_access_token(sa_json_path: str) -> str:
    with open(sa_json_path) as f:
        sa = json.load(f)

    now = int(time.time())
    header = {"alg": "RS256", "typ": "JWT"}
    claims = {
        "iss": sa["client_email"],
        "scope": SCOPE,
        "aud": sa["token_uri"],
        "exp": now + 3600,
        "iat": now,
    }
    signing_input = f"{b64url(json.dumps(header).encode())}.{b64url(json.dumps(claims).encode())}"

    with tempfile.NamedTemporaryFile(mode="w", suffix=".pem") as keyfile:
        keyfile.write(sa["private_key"])
        keyfile.flush()
        proc = subprocess.run(
            ["openssl", "dgst", "-sha256", "-sign", keyfile.name],
            input=signing_input.encode(),
            capture_output=True,
            check=True,
        )
    signature = b64url(proc.stdout)
    jwt = f"{signing_input}.{signature}"

    body = "&".join([
        "grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer",
        f"assertion={jwt}",
    ]).encode()
    req = urllib.request.Request(sa["token_uri"], data=body, method="POST")
    with urllib.request.urlopen(req) as resp:
        token_resp = json.loads(resp.read())
    return token_resp["access_token"]


def api(method: str, url: str, token: str, body: dict | None = None) -> dict:
    data = json.dumps(body).encode() if body is not None else (b"{}" if method == "POST" else None)
    req = urllib.request.Request(url, data=data, method=method)
    req.add_header("Authorization", f"Bearer {token}")
    req.add_header("Content-Type", "application/json")
    try:
        with urllib.request.urlopen(req) as resp:
            raw = resp.read()
            return json.loads(raw) if raw else {}
    except urllib.error.HTTPError as e:
        sys.stderr.write(f"HTTP {e.code} on {method} {url}: {e.read().decode()}\n")
        raise


def with_draft_edit(token: str, package: str, fn):
    base = f"https://androidpublisher.googleapis.com/androidpublisher/v3/applications/{package}/edits"
    edit = api("POST", base, token)
    edit_id = edit["id"]
    sys.stderr.write(f"Created draft edit: {edit_id}\n")
    try:
        return fn(base, edit_id)
    finally:
        del_url = f"{base}/{edit_id}"
        req = urllib.request.Request(del_url, method="DELETE")
        req.add_header("Authorization", f"Bearer {token}")
        try:
            urllib.request.urlopen(req)
            sys.stderr.write(f"Deleted draft edit {edit_id} (nothing was committed)\n")
        except urllib.error.HTTPError as e:
            sys.stderr.write(f"WARNING: failed to delete edit {edit_id}: {e}\n")


def next_version_code(sa_json_path: str, package: str) -> int:
    token = get_access_token(sa_json_path)

    def _fn(base, edit_id):
        tracks = api("GET", f"{base}/{edit_id}/tracks", token)
        codes = []
        for t in tracks.get("tracks", []):
            # Wear OS releases live under the same package but use a distinct
            # numbering range (10000+) precisely so they don't collide with the
            # phone app's own versionCode sequence - exclude them, we only care
            # about the phone app's next versionCode here.
            if t.get("track", "").startswith("wear"):
                sys.stderr.write(f"Skipping track '{t.get('track')}' (Wear OS, separate versionCode range)\n")
                continue
            for r in t.get("releases", []):
                for vc in r.get("versionCodes", []):
                    codes.append(int(vc))
        return max(codes) if codes else 0

    max_vc = with_draft_edit(token, package, _fn)
    sys.stderr.write(f"Max versionCode across all (non-Wear) tracks: {max_vc}\n")
    return max_vc + 1


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--sa-json", required=True)
    parser.add_argument("--package", default="org.cagnulen.qdomyoszwift")
    args = parser.parse_args()

    print(next_version_code(args.sa_json, args.package))
