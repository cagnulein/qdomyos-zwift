# iOS TestFlight release workflow

The GitHub Actions workflow **Release iOS to TestFlight** is a manual release path on `master`. The iOS Simulator CI job calls `.github/scripts/build_ios_simulator.sh` for pull requests. The release workflow applies the same generated-project preparation, then lets Fastlane create a signed archive for a physical iOS device; a Simulator build cannot be uploaded to App Store Connect.

The release archive must include **all four native Xcode targets**: `qdomyoszwift` (iPhone/iPad), `watchkit`, `watchkit Extension`, and `QZWidgetExtension`. Do not run `.github/scripts/ios_simulator_project.py` on the TestFlight release path: that script deliberately removes the Apple Watch target dependency and embed phase for the separate **iOS Simulator CI** build. The release workflow instead checks that the WatchKit dependency and both embed phases are retained. Fastlane then validates the **signed .xcarchive and exported .ipa before upload**, checking the nested Watch app, WatchKit extension, widget, bundle relationships and matching build numbers. A missing companion target fails the release rather than publishing an incomplete TestFlight build. This validation does not replace a physical iPhone/Apple Watch installation test.

Run it from **Actions → Release iOS to TestFlight → Run workflow** on `master`. Leave `publish` unchecked to create a signed IPA artifact only. Check it to upload the IPA to TestFlight.

To **check the signed archive without uploading**, add the `ios-archive-check` label to a same-repository pull request. This runs the complete signed device archive/export and bundled-target verification, and retains the IPA as an Actions artifact. Once the archive is validated, add the `testflight-upload` label to build and upload a TestFlight beta. Both labels apply only to same-repository pull requests targeting `master`; fork pull requests cannot use App Store Connect secrets. Other pull request updates do not upload automatically.

## GitHub Actions secrets

The workflow uses these App Store Connect API key secrets:

| Secret | Value |
| --- | --- |
| `ASC_KEY_ID` | Key ID from App Store Connect |
| `ASC_ISSUER_ID` | Issuer ID from App Store Connect |
| `ASC_API_KEY_BASE64` | Base64-encoded contents of the downloaded `.p8` private key |

Fastlane uses the API key to query the latest TestFlight build number and upload the IPA. Xcode also uses it with automatic signing and `-allowProvisioningUpdates` to retrieve the provisioning profiles and cloud-sign the app for distribution. Keep the key in GitHub Actions secrets; do not commit the `.p8` file.

The temporary GitHub runner needs the same **Apple Development** identity that Xcode uses to create the archive. The Apple Developer team is `6335M7T29D`. Xcode can use the cloud-managed Apple Distribution certificate for the later distribution step; that certificate does not have an exportable private key. Export the Apple Development identity and its private key together as a password-protected `.p12` from Xcode's **Manage Certificates** sheet, then Base64-encode it and add these repository secrets:

| Secret | Value |
| --- | --- |
| `IOS_DEVELOPMENT_CERTIFICATE_BASE64` | Base64-encoded `.p12` containing the Apple Development certificate and private key for team `6335M7T29D` |
| `IOS_DEVELOPMENT_CERTIFICATE_PASSWORD` | Password used to export the `.p12` |

Add them under the repository's **Settings → Secrets and variables → Actions**. On macOS, encode the exported identity and copy it to the clipboard:

```sh
base64 -i signing-development.p12 | tr -d '\n' | pbcopy
```

The workflow imports the identity into a temporary keychain and removes it at the end of the job. Automatic signing then reuses the existing development certificate instead of asking Apple to create a new one on each temporary runner. Do not commit the `.p12` file.

The Xcode Cloud setup script also reads the existing GitHub secrets used by the iOS build: `strava_secret_key`, `peloton_secret_key`, `smtp_username`, `smtp_password`, `smtp_server`, `intervalsicu_client_id`, `intervalsicu_client_secret`, and `cesiumkey`.

If App Store Connect shows an updated Apple Developer Program License Agreement, the Account Holder must accept it before a new build can be submitted.

## Export compliance

The iOS app bundle declares `ITSAppUsesNonExemptEncryption` as `false` in `src/ios/Info.plist`, which is the `INFOPLIST_FILE` used by the signed app target. This corresponds to the App Store Connect selection that the app uses no encryption requiring export documentation (no encryption or exempt encryption). App Store Connect reads the declaration from each new IPA and should no longer request the same TestFlight export-compliance answer for each upload.

This is a legal classification, not a Fastlane workaround. Reassess it if the app or its linked libraries begin using cryptography that is not exempt; update the plist value and provide documentation when Apple requires it. After building, confirm the exported IPA contains `ITSAppUsesNonExemptEncryption = false` in `Payload/qdomyoszwift.app/Info.plist`, and verify the next processed TestFlight build does not show **Missing Compliance**.

## Build numbering and signing

Fastlane reads the current iOS version/build settings, queries the latest TestFlight build for that version, and uses the next build number for the archive. It archives with the persistent Apple Development identity, then Xcode cloud-signs for TestFlight distribution. The workflow retains the exported IPA as a 14-day Actions artifact.
