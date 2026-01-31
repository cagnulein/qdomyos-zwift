# iOS App Store Workflow Setup

This document explains how to configure the GitHub Actions workflow for building and publishing the iOS app to App Store Connect.

## Prerequisites

1. **Apple Developer Account** with App Store Connect access
2. **Distribution Certificate** (.p12 file with private key)
3. **Provisioning Profiles** for App Store distribution
4. **App Store Connect API Key** for automated uploads

## Required GitHub Secrets

Go to your repository's **Settings → Secrets and variables → Actions** and add the following secrets:

### Certificate & Signing

| Secret Name | Description | How to Get It |
|-------------|-------------|---------------|
| `APPLE_CERTIFICATE_P12` | Base64 encoded distribution certificate | See "Exporting Certificate" below |
| `APPLE_CERTIFICATE_PASSWORD` | Password for the .p12 file | Password you set when exporting |
| `KEYCHAIN_PASSWORD` | Any secure password for temp keychain | Generate a random password |

### Provisioning Profiles

| Secret Name | Description | How to Get It |
|-------------|-------------|---------------|
| `APPLE_PROVISIONING_PROFILE` | Base64 encoded main app profile | Download from Apple Developer Portal |
| `APPLE_PROVISIONING_PROFILE_WATCHKIT` | Base64 encoded WatchKit app profile | Download from Apple Developer Portal |
| `APPLE_PROVISIONING_PROFILE_WATCHKIT_EXTENSION` | Base64 encoded WatchKit Extension profile | Download from Apple Developer Portal |

### App Store Connect API

| Secret Name | Description | How to Get It |
|-------------|-------------|---------------|
| `APP_STORE_CONNECT_API_KEY_ID` | API Key ID (e.g., "ABC123DEFG") | App Store Connect → Users and Access → Keys |
| `APP_STORE_CONNECT_API_ISSUER_ID` | Issuer ID (UUID format) | App Store Connect → Users and Access → Keys |
| `APP_STORE_CONNECT_API_KEY_P8` | Base64 encoded API Key (.p8 file) | See "Creating API Key" below |

## Step-by-Step Setup

### 1. Exporting Distribution Certificate

1. Open **Keychain Access** on your Mac
2. Find your "Apple Distribution" or "iPhone Distribution" certificate
3. Right-click → **Export** → Save as .p12 with a password
4. Convert to Base64:
   ```bash
   base64 -i certificate.p12 | pbcopy
   ```
5. Paste the result into `APPLE_CERTIFICATE_P12` secret

### 2. Getting Provisioning Profiles

1. Go to [Apple Developer Portal](https://developer.apple.com/account/resources/profiles/list)
2. Download the **App Store** distribution profiles for:
   - Main app: `org.cagnulein.qdomyoszwift`
   - WatchKit app: `org.cagnulein.qdomyoszwift.watchkitapp`
   - WatchKit Extension: `org.cagnulein.qdomyoszwift.watchkitapp.watchkitextension`
3. Convert each to Base64:
   ```bash
   base64 -i profile.mobileprovision | pbcopy
   ```
4. Paste into the corresponding secrets

### 3. Creating App Store Connect API Key

1. Go to [App Store Connect → Users and Access → Integrations → App Store Connect API](https://appstoreconnect.apple.com/access/integrations/api)
2. Click **+** to create a new key
3. Give it a name (e.g., "GitHub Actions")
4. Select **Admin** or **App Manager** access
5. Download the .p8 file (you can only download it once!)
6. Note the **Key ID** and **Issuer ID**
7. Convert the .p8 to Base64:
   ```bash
   base64 -i AuthKey_XXXXXXXX.p8 | pbcopy
   ```
8. Add all three values to secrets

### 4. Existing Secrets (Already Configured)

These secrets should already exist for the CI build:
- `strava_secret_key`
- `peloton_secret_key`
- `smtp_username`
- `smtp_password`
- `smtp_server`
- `intervalsicu_client_id`
- `intervalsicu_client_secret`
- `cesiumkey`

## Running the Workflow

1. Go to **Actions** tab in your repository
2. Select **iOS App Store** workflow
3. Click **Run workflow**
4. Options:
   - **Upload to TestFlight**: Check to automatically upload after build
   - **Build number increment**: Enter a new build number (optional)

## Workflow Outputs

- **IPA file**: Downloaded as artifact, stored for 30 days
- **dSYM files**: For crash reporting, stored for 30 days
- **TestFlight upload**: Automatic if selected

## Troubleshooting

### "No signing certificate found"
- Verify the certificate is not expired
- Ensure it's a Distribution certificate (not Development)
- Check the Base64 encoding is correct

### "Provisioning profile doesn't match"
- Download fresh profiles from Apple Developer Portal
- Ensure profiles are for App Store distribution
- Verify bundle IDs match

### "Invalid API Key"
- Regenerate the key in App Store Connect
- Check Key ID and Issuer ID are correct
- Ensure the key has sufficient permissions

## Manual Alternative

If you prefer the manual process:

1. Run the workflow with "Upload to TestFlight" unchecked
2. Download the IPA artifact
3. Use **Transporter** app on Mac to upload manually

## Build Number Management

The workflow can automatically update the build number. Enter a new build number in the workflow input, or leave empty to keep the current version from `project.pbxproj`.

Current version info in project:
- Marketing Version: 2.20
- Current Build Number: 1274
