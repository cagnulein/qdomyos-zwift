# iOS TestFlight release workflow

The GitHub Actions workflow **Release iOS to TestFlight** is a manual release path on `master`. It runs the same `ci_post_clone.sh` and `ci_pre_xcodebuild.sh` scripts as the iOS Simulator job, then uses Fastlane to archive for a physical iOS device. The Simulator build itself cannot be uploaded to App Store Connect.

Run it from **Actions → Release iOS to TestFlight → Run workflow**. Leave `publish` unchecked to create a signed IPA artifact only. Check it to upload the IPA to TestFlight. The workflow does not run for pushes or pull requests.

## GitHub Actions secrets

The workflow uses these App Store Connect API key secrets:

| Secret | Value |
| --- | --- |
| `ASC_KEY_ID` | Key ID from App Store Connect |
| `ASC_ISSUER_ID` | Issuer ID from App Store Connect |
| `ASC_API_KEY_BASE64` | Base64-encoded contents of the downloaded `.p8` private key |

The key is used by Xcode automatic signing and by Fastlane for the App Store Connect API. Keep it in GitHub Actions secrets; do not commit the `.p8` file. A Team API key with the minimum role that supports automatic signing and binary uploads should be used.

The Xcode Cloud setup script also reads the existing GitHub secrets used by the iOS build: `strava_secret_key`, `peloton_secret_key`, `smtp_username`, `smtp_password`, `smtp_server`, `intervalsicu_client_id`, `intervalsicu_client_secret`, and `cesiumkey`.

If App Store Connect shows an updated Apple Developer Program License Agreement, the Account Holder must accept it before a new build can be submitted.

## Build numbering and signing

Fastlane reads the current iOS version/build settings, queries the latest TestFlight build for that version, and uses the next build number for the archive. Xcode automatic signing uses the API key and the developer team already configured in the Xcode project. The workflow retains the exported IPA as a 14-day Actions artifact.
