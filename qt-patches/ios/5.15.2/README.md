# Qt 5.15.2 iOS UIScene backport

This directory contains the Qt 5.15.2 iOS backports required by the Xcode 27
SDK when running the iPad target on macOS. It changes only Qt's platform
plugins; QDomyos-Zwift sources and `Info.plist` are unchanged.

The source patch is based on Qt's UIScene work in commit
`76ebf51bc08f6af624a8540e7af88b9129b22ae1`.

Apply the patch from the Qt base source root:

```sh
git apply -p1 qt-patches/ios/5.15.2/qios-uiscene.patch
```

Build the plugin with `tools/build_qios_uiscene.sh`. Set `BUILD_CONFIG=debug`
to create the debug variant. The script defaults to the local Qt 5.15.2 iOS
installation and the patched source directory used during development.

The QtBluetooth backend also contains a small Xcode 27 compatibility guard in
`qtconnectivity/src/bluetooth/osx/osxbtcentralmanager.mm`: CoreBluetooth can
deliver a duplicate connection request while the manager is busy, which was a
fatal assertion in a Debug build on “My Mac (Designed for iPad)”. The matching
release and Debug archives are versioned under `binary/`.
