# Garmin Connect OAuth Setup

This document explains how to configure OAuth consumer credentials for Garmin Connect authentication in qdomyos-zwift.

## Background

The Garmin Connect authentication flow requires OAuth consumer credentials (key and secret) to perform OAuth1 authentication. The Python [garth library](https://github.com/matin/garth) fetches these credentials from an S3 bucket at:
```
https://thegarth.s3.amazonaws.com/oauth_consumer.json
```

However, this S3 bucket may sometimes be restricted or inaccessible due to network policies, firewall rules, or access restrictions.

## Automatic Mode (Default)

By default, qdomyos-zwift will attempt to fetch OAuth consumer credentials from the S3 URL automatically. If this works, no additional configuration is needed.

## Manual Configuration (If S3 is Blocked)

If you see errors like:
- `"Invalid OAuth consumer response"`
- `"Failed to fetch OAuth consumer (HTTP 403)"`
- `"This S3 URL may be restricted or no longer accessible"`

You can provide OAuth consumer credentials manually using environment variables.

### Method 1: Using the Helper Script (Recommended)

We provide a Python script that fetches the credentials and displays the export commands:

```bash
# Run the helper script
python3 extract_garth_credentials.py

# Follow the instructions in the output to set environment variables
```

The script will output commands like:
```bash
export GARMIN_OAUTH_CONSUMER_KEY="your_key_here"
export GARMIN_OAUTH_CONSUMER_SECRET="your_secret_here"
```

### Method 2: Extract from Python garth Installation

If you have Python garth installed and working:

```python
import requests

response = requests.get("https://thegarth.s3.amazonaws.com/oauth_consumer.json")
credentials = response.json()

print(f"Key:    {credentials['consumer_key']}")
print(f"Secret: {credentials['consumer_secret']}")
```

### Method 3: Extract from garth Source Code or Network Capture

1. **From garth source**: Check if credentials are embedded in the garth Python package installed on your system
2. **Network capture**: Use a network monitoring tool (tcpdump, Wireshark) to capture the S3 request from a working Python garth session

## Setting Environment Variables

### Temporary (Current Session Only)

```bash
export GARMIN_OAUTH_CONSUMER_KEY="your_key_here"
export GARMIN_OAUTH_CONSUMER_SECRET="your_secret_here"
./qdomyos-zwift
```

### Permanent (Recommended)

Add the export commands to your shell configuration file:

**For bash** (~/.bashrc):
```bash
echo 'export GARMIN_OAUTH_CONSUMER_KEY="your_key_here"' >> ~/.bashrc
echo 'export GARMIN_OAUTH_CONSUMER_SECRET="your_secret_here"' >> ~/.bashrc
source ~/.bashrc
```

**For zsh** (~/.zshrc):
```bash
echo 'export GARMIN_OAUTH_CONSUMER_KEY="your_key_here"' >> ~/.zshrc
echo 'export GARMIN_OAUTH_CONSUMER_SECRET="your_secret_here"' >> ~/.zshrc
source ~/.zshrc
```

## Verification

When qdomyos-zwift runs, check the log output:
- ✅ `"Using OAuth consumer credentials from environment variables"` - Manual configuration working
- ✅ `"OAuth consumer response received, length: XXX"` - Automatic S3 fetch working
- ❌ `"Failed to fetch OAuth consumer"` - Configuration needed

## Security Note

OAuth consumer credentials are **public** credentials used by all Garmin Connect API clients. They are not secret in the same way as your personal login credentials (email/password). However, you should still:
- Not commit them to public repositories
- Store them securely in environment variables or configuration files with appropriate permissions

## Troubleshooting

### Script fails with "requests library not found"
```bash
pip3 install requests
```

### S3 URL returns 403 Forbidden
This is the expected scenario this workaround addresses. Use the environment variable method above.

### Environment variables not being recognized
- Make sure to `source` your shell configuration file after editing
- Verify variables are set: `echo $GARMIN_OAUTH_CONSUMER_KEY`
- Make sure there are no spaces around the `=` in export commands

### Still getting authentication errors after setting variables
Check the qdomyos-zwift logs for:
1. Confirmation that environment variables are being used
2. The full error message from Garmin's OAuth endpoint
3. Cookie and CSRF token extraction status

## Related Files

- `src/garminconnect.h` - GarminConnect class header
- `src/garminconnect.cpp` - Implementation with environment variable support
- `extract_garth_credentials.py` - Helper script to fetch credentials
- `PYTHON_GARTH_OAUTH1_DETAILED_ANALYSIS.md` - Technical analysis of OAuth flow

## References

- [garth library](https://github.com/matin/garth) - Original Python implementation
- [Garmin Connect API](https://connect.garmin.com/) - Official Garmin Connect platform
