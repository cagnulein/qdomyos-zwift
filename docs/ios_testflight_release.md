# iOS TestFlight release workflow

The GitHub Actions workflow **Release iOS to TestFlight** is a manual release path on `master`. The iOS Simulator CI job calls `.github/scripts/build_ios_simulator.sh` for pull requests. The release workflow applies the same generated-project preparation, then lets Fastlane create a signed archive for a physical iOS device; a Simulator build cannot be uploaded to App Store Connect.

Run it from **Actions → Release iOS to TestFlight → Run workflow** on `master`. Leave `publish` unchecked to create a signed IPA artifact only. Check it to upload the IPA to TestFlight.

To test an open pull request before merging, add the `testflight-upload` label. This label triggers the signed device archive and TestFlight upload for same-repository pull requests targeting `master`. Other pull request updates do not upload automatically, and fork pull requests cannot use the App Store Connect secrets.

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

## Export compliance

The iOS app bundle declares `ITSAppUsesNonExemptEncryption` as `false` in `src/ios/Info.plist`, which is the `INFOPLIST_FILE` used by the signed app target. This corresponds to the App Store Connect selection that the app uses no encryption requiring export documentation (no encryption or exempt encryption). App Store Connect reads the declaration from each new IPA and should no longer request the same TestFlight export-compliance answer for each upload.

This is a legal classification, not a Fastlane workaround. Reassess it if the app or its linked libraries begin using cryptography that is not exempt; update the plist value and provide documentation when Apple requires it. After building, confirm the exported IPA contains `ITSAppUsesNonExemptEncryption = false` in `Payload/qdomyoszwift.app/Info.plist`, and verify the next processed TestFlight build does not show **Missing Compliance**.

## Build numbering and signing

Fastlane reads the current iOS version/build settings, queries the latest TestFlight build for that version, and uses the next build number for the archive. Xcode automatic signing uses the API key and the developer team already configured in the Xcode project. The workflow retains the exported IPA as a 14-day Actions artifact.
