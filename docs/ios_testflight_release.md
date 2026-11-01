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

Fastlane uses the API key to query the latest TestFlight build number and upload the IPA. Keep it in GitHub Actions secrets; do not commit the `.p8` file. Code signing uses the persistent certificate and provisioning profiles described below, so the workflow does not ask Xcode to create signing assets on the temporary runner.

The signing assets must be for the Apple Developer team `6335M7T29D` and must include the app's entitlements. Reuse an Apple Distribution certificate for which the private key can be exported; Apple allows only one certificate of each distribution type per team ([certificates overview](https://developer.apple.com/help/account/certificates/certificates-overview)). Create App Store provisioning profiles for both bundle IDs:

- `org.cagnulein.qdomyoszwift` (app; includes HealthKit, multicast networking, the `group.org.cagnulein.qdomyoszwift` app group, and the `applinks:www.qzfitness.com` associated domain)
- `org.cagnulein.qdomyoszwift.QZWidget` (widget extension; includes the `group.org.cagnulein.qdomyoszwift` app group)

Export the existing Apple Distribution certificate and its private key together as a password-protected `.p12` from the Mac/keychain that owns the private key. A certificate download from the Apple Developer portal does not include its private key. Base64-encode that file and each `.mobileprovision` profile, then add these repository secrets:

| Secret | Value |
| --- | --- |
| `IOS_SIGNING_CERTIFICATE_BASE64` | Base64-encoded `.p12` containing the Apple Distribution certificate and private key |
| `IOS_SIGNING_CERTIFICATE_PASSWORD` | Password used to export the `.p12` |
| `IOS_APP_PROVISIONING_PROFILE_BASE64` | Base64-encoded App Store profile for `org.cagnulein.qdomyoszwift` |
| `IOS_WIDGET_PROVISIONING_PROFILE_BASE64` | Base64-encoded App Store profile for `org.cagnulein.qdomyoszwift.QZWidget` |

Add them under the repository's **Settings → Secrets and variables → Actions**. On macOS, run one command at a time, paste the clipboard into its matching secret, then run the next command:

```sh
base64 -i signing.p12 | tr -d '\n' | pbcopy
base64 -i app.mobileprovision | tr -d '\n' | pbcopy
base64 -i widget.mobileprovision | tr -d '\n' | pbcopy
```

The workflow imports the certificate into a temporary keychain, installs both profiles, signs the app and widget with manual signing, and removes the temporary signing material at the end of the job. Do not commit the `.p12` or `.mobileprovision` files.

The Xcode Cloud setup script also reads the existing GitHub secrets used by the iOS build: `strava_secret_key`, `peloton_secret_key`, `smtp_username`, `smtp_password`, `smtp_server`, `intervalsicu_client_id`, `intervalsicu_client_secret`, and `cesiumkey`.

If App Store Connect shows an updated Apple Developer Program License Agreement, the Account Holder must accept it before a new build can be submitted.

## Export compliance

The iOS app bundle declares `ITSAppUsesNonExemptEncryption` as `false` in `src/ios/Info.plist`, which is the `INFOPLIST_FILE` used by the signed app target. This corresponds to the App Store Connect selection that the app uses no encryption requiring export documentation (no encryption or exempt encryption). App Store Connect reads the declaration from each new IPA and should no longer request the same TestFlight export-compliance answer for each upload.

This is a legal classification, not a Fastlane workaround. Reassess it if the app or its linked libraries begin using cryptography that is not exempt; update the plist value and provide documentation when Apple requires it. After building, confirm the exported IPA contains `ITSAppUsesNonExemptEncryption = false` in `Payload/qdomyoszwift.app/Info.plist`, and verify the next processed TestFlight build does not show **Missing Compliance**.

## Build numbering and signing

Fastlane reads the current iOS version/build settings, queries the latest TestFlight build for that version, and uses the next build number for the archive. It signs with the Apple Distribution identity and the two installed App Store profiles. The workflow retains the exported IPA as a 14-day Actions artifact.
