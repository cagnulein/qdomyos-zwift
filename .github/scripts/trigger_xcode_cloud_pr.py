#!/usr/bin/env python3

import json
import os
import sys
import time
import urllib.error
import urllib.parse
import urllib.request

import jwt


API_BASE = "https://api.appstoreconnect.apple.com"


def fail(message):
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def request_json(token, url, method="GET", body=None, headers=None):
    if not url.startswith("http"):
        url = API_BASE + url

    request_headers = {"Accept": "application/json"}
    if url.startswith(API_BASE):
        request_headers["Authorization"] = f"Bearer {token}"
    if headers:
        request_headers.update(headers)

    data = None
    if body is not None:
        data = json.dumps(body).encode("utf-8")
        request_headers["Content-Type"] = "application/json"

    req = urllib.request.Request(
        url,
        data=data,
        headers=request_headers,
        method=method,
    )

    try:
        with urllib.request.urlopen(req, timeout=30) as response:
            payload = response.read().decode("utf-8")
            return json.loads(payload) if payload else {}
    except urllib.error.HTTPError as exc:
        details = exc.read().decode("utf-8", errors="replace")
        print(f"HTTP {exc.code} calling {url}", file=sys.stderr)
        print(details, file=sys.stderr)
        raise


def github_pr_metadata(github_repo, pr_number, github_token):
    return request_json(
        token="",
        url=f"https://api.github.com/repos/{github_repo}/pulls/{pr_number}",
        headers={
            "Authorization": f"Bearer {github_token}",
            "Accept": "application/vnd.github+json",
            "X-GitHub-Api-Version": "2022-11-28",
        },
    )


def expected_pr_attributes(gh_pr):
    head_repo = gh_pr["head"]["repo"]
    base_repo = gh_pr["base"]["repo"]
    return {
        "webUrl": gh_pr["html_url"].rstrip("/"),
        "sourceRepositoryOwner": head_repo["owner"]["login"],
        "sourceRepositoryName": head_repo["name"],
        "sourceBranchName": gh_pr["head"]["ref"],
        "destinationRepositoryOwner": base_repo["owner"]["login"],
        "destinationRepositoryName": base_repo["name"],
        "destinationBranchName": gh_pr["base"]["ref"],
    }


def same_pr(candidate, expected, pr_number):
    attrs = candidate.get("attributes", {})

    same_url = attrs.get("webUrl", "").rstrip("/") == expected["webUrl"]
    same_metadata = (
        attrs.get("number") == pr_number
        and attrs.get("sourceRepositoryOwner", "").lower()
        == expected["sourceRepositoryOwner"].lower()
        and attrs.get("sourceRepositoryName", "").lower()
        == expected["sourceRepositoryName"].lower()
        and attrs.get("sourceBranchName") == expected["sourceBranchName"]
        and attrs.get("destinationRepositoryOwner", "").lower()
        == expected["destinationRepositoryOwner"].lower()
        and attrs.get("destinationRepositoryName", "").lower()
        == expected["destinationRepositoryName"].lower()
        and attrs.get("destinationBranchName") == expected["destinationBranchName"]
    )
    return (same_url or same_metadata) and not attrs.get("isClosed", False)


def find_apple_pull_request(token, scm_repo_id, expected, pr_number):
    fields = ",".join(
        [
            "title",
            "number",
            "webUrl",
            "sourceRepositoryOwner",
            "sourceRepositoryName",
            "sourceBranchName",
            "destinationRepositoryOwner",
            "destinationRepositoryName",
            "destinationBranchName",
            "isClosed",
            "isCrossRepository",
        ]
    )
    query = urllib.parse.urlencode(
        {
            "limit": "200",
            "fields[scmPullRequests]": fields,
        }
    )
    next_url = (
        f"{API_BASE}/v1/scmRepositories/{urllib.parse.quote(scm_repo_id)}"
        f"/pullRequests?{query}"
    )

    while next_url:
        page = request_json(token, next_url)
        for candidate in page.get("data", []):
            if same_pr(candidate, expected, pr_number):
                return candidate
        next_url = page.get("links", {}).get("next")

    return None


def append_summary(pr_number, apple_pr, build_data):
    summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if not summary:
        return

    attrs = apple_pr.get("attributes", {})
    build_id = build_data["id"]
    build_number = build_data.get("attributes", {}).get("number")

    with open(summary, "a", encoding="utf-8") as handle:
        handle.write("## Xcode Cloud build started\n\n")
        handle.write(f"- GitHub PR: #{pr_number}\n")
        handle.write(
            f"- Cross-repository PR: {attrs.get('isCrossRepository', False)}\n"
        )
        if build_number is not None:
            handle.write(f"- Xcode Cloud build: #{build_number}\n")
        handle.write(f"- Build run ID: `{build_id}`\n")


def main():
    pr_number = int(os.environ["PR_NUMBER"])
    workflow_id = os.environ["XCODE_CLOUD_WORKFLOW_ID"].strip()
    github_repo = os.environ["GH_REPOSITORY"]
    github_token = os.environ["GH_TOKEN"]

    private_key = os.environ["ASC_PRIVATE_KEY"]
    if "\\n" in private_key and "\n" not in private_key:
        private_key = private_key.replace("\\n", "\n")

    now = int(time.time())
    token = jwt.encode(
        {
            "iss": os.environ["ASC_ISSUER_ID"].strip(),
            "iat": now,
            "exp": now + 600,
            "aud": "appstoreconnect-v1",
        },
        private_key,
        algorithm="ES256",
        headers={
            "kid": os.environ["ASC_KEY_ID"].strip(),
            "typ": "JWT",
        },
    )

    gh_pr = github_pr_metadata(github_repo, pr_number, github_token)
    if gh_pr.get("state") != "open":
        fail(f"PR #{pr_number} is not open")

    expected = expected_pr_attributes(gh_pr)

    repo_info = request_json(
        token,
        f"/v1/ciWorkflows/{urllib.parse.quote(workflow_id)}/repository"
        "?fields%5BscmRepositories%5D=ownerName,repositoryName",
    )
    scm_repo = repo_info["data"]
    scm_repo_id = scm_repo["id"]
    scm_attrs = scm_repo.get("attributes", {})

    if (
        scm_attrs.get("ownerName", "").lower()
        != expected["destinationRepositoryOwner"].lower()
        or scm_attrs.get("repositoryName", "").lower()
        != expected["destinationRepositoryName"].lower()
    ):
        fail(
            "Configured Xcode Cloud workflow belongs to "
            f"{scm_attrs.get('ownerName')}/{scm_attrs.get('repositoryName')}, "
            f"but PR #{pr_number} targets "
            f"{expected['destinationRepositoryOwner']}/"
            f"{expected['destinationRepositoryName']}"
        )

    apple_pr = find_apple_pull_request(
        token,
        scm_repo_id,
        expected,
        pr_number,
    )
    if not apple_pr:
        fail(
            f"Xcode Cloud does not currently expose PR #{pr_number} for the "
            "repository connected to this workflow. Make sure the Xcode Cloud "
            "GitHub connection can access that pull request/fork."
        )

    attrs = apple_pr.get("attributes", {})
    print(
        "Found Xcode Cloud PR "
        f"#{attrs.get('number')} "
        f"{attrs.get('sourceRepositoryOwner')}/"
        f"{attrs.get('sourceRepositoryName')}:"
        f"{attrs.get('sourceBranchName')} -> "
        f"{attrs.get('destinationRepositoryOwner')}/"
        f"{attrs.get('destinationRepositoryName')}:"
        f"{attrs.get('destinationBranchName')} "
        f"(cross-repository={attrs.get('isCrossRepository')})"
    )

    build = request_json(
        token,
        "/v1/ciBuildRuns",
        method="POST",
        body={
            "data": {
                "type": "ciBuildRuns",
                "attributes": {},
                "relationships": {
                    "workflow": {
                        "data": {
                            "type": "ciWorkflows",
                            "id": workflow_id,
                        }
                    },
                    "pullRequest": {
                        "data": {
                            "type": "scmPullRequests",
                            "id": apple_pr["id"],
                        }
                    },
                },
            }
        },
    )

    build_data = build["data"]
    build_id = build_data["id"]
    build_number = build_data.get("attributes", {}).get("number")
    print(
        "Started Xcode Cloud build"
        + (f" #{build_number}" if build_number is not None else "")
        + f" (build run ID: {build_id})"
    )

    append_summary(pr_number, apple_pr, build_data)


if __name__ == "__main__":
    main()
