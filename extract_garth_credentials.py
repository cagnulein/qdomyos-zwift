#!/usr/bin/env python3
"""
Helper script to extract OAuth consumer credentials from Python garth library.

This script fetches the OAuth consumer credentials that garth uses and displays
them so you can set them as environment variables for the C++ Qt implementation.

Usage:
    python3 extract_garth_credentials.py

Then set the environment variables before running qdomyos-zwift:
    export GARMIN_OAUTH_CONSUMER_KEY="<key from output>"
    export GARMIN_OAUTH_CONSUMER_SECRET="<secret from output>"
    ./qdomyos-zwift
"""

import sys

try:
    import requests
except ImportError:
    print("ERROR: requests library not found. Install with: pip3 install requests")
    sys.exit(1)

OAUTH_CONSUMER_URL = "https://thegarth.s3.amazonaws.com/oauth_consumer.json"

def main():
    print("Fetching OAuth consumer credentials from garth S3...")
    print(f"URL: {OAUTH_CONSUMER_URL}\n")

    try:
        response = requests.get(OAUTH_CONSUMER_URL)
        response.raise_for_status()

        credentials = response.json()

        consumer_key = credentials.get("consumer_key", "")
        consumer_secret = credentials.get("consumer_secret", "")

        if not consumer_key or not consumer_secret:
            print("ERROR: Response does not contain required fields")
            print(f"Response: {credentials}")
            sys.exit(1)

        print("SUCCESS! OAuth consumer credentials retrieved:\n")
        print(f"Consumer Key:    {consumer_key}")
        print(f"Consumer Secret: {consumer_secret}\n")
        print("=" * 70)
        print("To use these with qdomyos-zwift, set these environment variables:\n")
        print(f'export GARMIN_OAUTH_CONSUMER_KEY="{consumer_key}"')
        print(f'export GARMIN_OAUTH_CONSUMER_SECRET="{consumer_secret}"')
        print("\nOr add them to your ~/.bashrc or ~/.zshrc to make them permanent.")
        print("=" * 70)

    except requests.exceptions.RequestException as e:
        print(f"ERROR: Failed to fetch credentials: {e}")
        print(f"\nHTTP Status: {getattr(e.response, 'status_code', 'N/A')}")
        if hasattr(e, 'response') and e.response is not None:
            print(f"Response Body: {e.response.text[:500]}")
        print("\nIf you get a 403 Forbidden error, the S3 bucket may be restricted.")
        print("You may need to extract credentials from a working Python garth installation.")
        sys.exit(1)

if __name__ == "__main__":
    main()
