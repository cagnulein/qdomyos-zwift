# Qt 5.15.2 iOS UIScene backport

This directory contains the Qt 5.15.2 iOS backports required by the Xcode 27
SDK when running the iPad target on macOS. It changes only Qt's platform
plugins in the historical backport path; the current QZ integration is kept
in a project-level Objective-C++ overlay.

The current QZ application uses the UIScene compatibility overlay in
`src/ios/ios_app_delegate.mm`; the `libqios*.a` archives kept in `binary/` are
unmodified Qt archives. The source patches below are retained as historical
rebuild references and are not part of the normal QZ build.

The QtBluetooth archives remain the existing Xcode 27-compatible package
component and are intentionally unchanged in this overlay migration.

The source patch is based on Qt's UIScene work in commit
`76ebf51bc08f6af624a8540e7af88b9129b22ae1`.

Apply the patch from the Qt base source root:

```sh
git apply -p1 qt-patches/ios/5.15.2/qios-uiscene.patch
```

Build the plugin with `tools/build_qios_uiscene.sh`. Set `BUILD_CONFIG=debug`
to create the debug variant. The script defaults to the local Qt 5.15.2 iOS
installation and the patched source directory used during development.

To keep the original Qt source tree untouched, point `QTBASE_SOURCE_DIR` at a
clean Qt source tree and set `APPLY_PATCHES=1`. The script copies that tree to
a temporary overlay, applies the repository patch there, and builds from the
overlay:

```sh
QTBASE_SOURCE_DIR=/path/to/qt-everywhere-src-5.15.2 \
APPLY_PATCHES=1 BUILD_CONFIG=debug \
tools/build_qios_uiscene.sh
```

The overlay is temporary; only the resulting archive is copied to `binary/`.
The QZ Xcode target must still link that produced archive because adding the
Qt platform source files directly to the QZ target would create duplicate
plugin symbols.

The resulting QPA archives are also versioned under `binary/` as
`libqios.a` and `libqios_debug.a`. Xcode Cloud normally gets the release
`libqios.a` from the Qt iOS27 package; keeping both archives here makes the
patch directory self-contained for local Debug and Release builds.

The QtBluetooth backend also contains a small Xcode 27 compatibility guard in
`qtconnectivity/src/bluetooth/osx/osxbtcentralmanager.mm`: CoreBluetooth can
deliver a duplicate connection request while the manager is busy, which was a
fatal assertion in a Debug build on “My Mac (Designed for iPad)”. The matching
release and Debug archives are versioned under `binary/`.
