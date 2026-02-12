brew install qt@5
#awk '
#    /\/\* Begin PBXShellScriptBuildPhase section \*\// {p=1}
#    /\/\* End PBXShellScriptBuildPhase section \*\// {p=0; next}
#    !p
#' /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj > /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj
#sed -i '' '/5E618435888B9D49F8540165 \/\* Qt Qmake \*\*,/d' /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj
#sed -i '' 's|/Users/cagnulein/Qt/5.15.2/ios/|/usr/local/opt/qt@5/|g' /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj

#cat /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj

PATH="/usr/local/opt/qt@5/bin:$PATH"

# Get Qt installation path dynamically
QT_PATH=$(brew --prefix qt@5)

ls -ltr /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS17.0.sdk/System/Library/Frameworks/OpenGL.framework/Headers

cd /Volumes/workspace/repository/src/qthttpserver
qmake
make -j8
make install

cd /Volumes/workspace/repository/
qmake -spec macx-ios-clang CONFIG+=iphoneos CONFIG+=device CONFIG+=qml_debug -after
#make -j8
#make install

mkdir -p "$QT_PATH/include/QtHttpServer"
mkdir -p "$QT_PATH/include/QtSslServer"
cp -a "$QT_PATH/lib/QtHttpServer.framework/Versions/5/Headers/"* "$QT_PATH/include/QtHttpServer/"
cp -a "$QT_PATH/lib/QtSslServer.framework/Versions/5/Headers/"* "$QT_PATH/include/QtSslServer/"

ls -ltr "$QT_PATH/include/QtHttpServer/"

echo "=== SCRIPT VERSION: 2026-02-12-v2 - Restoring project and workspace settings ==="

# Restore the good Xcode project and workspace settings from repository (after qmake overwrites them)
git checkout -- /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj
git checkout -- /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.xcworkspace/xcshareddata/

echo "=== Git checkout completed ==="

#sed -i '' 's|/Users/cagnulein/qdomyos-zwift|..|g' /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj
#add "" for folder qt@5
