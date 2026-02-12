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

ls -ltr /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS17.0.sdk/System/Library/Frameworks/OpenGL.framework/Headers

cd /Volumes/workspace/repository/src/qthttpserver 
qmake
make -j8
make install

cd /Volumes/workspace/repository/
qmake -spec macx-ios-clang CONFIG+=iphoneos CONFIG+=device CONFIG+=qml_debug -after
#make -j8
#make install

mkdir /usr/local/Cellar/qt@5/5.15.10_1/include/QtHttpServer
mkdir /usr/local/Cellar/qt@5/5.15.10_1/include/QtSslServer
cp -a /usr/local/Cellar/qt@5/5.15.10_1/lib/QtHttpServer.framework/Versions/5/Headers/* /usr/local/Cellar/qt@5/5.15.10_1/include/QtHttpServer/
cp -a /usr/local/Cellar/qt@5/5.15.10_1/lib/QtSslServer.framework/Versions/5/Headers/* /usr/local/Cellar/qt@5/5.15.10_1/include/QtSslServer/

ls -ltr /usr/local/Cellar/qt@5/5.15.10_1/include/QtHttpServer/

git checkout -- /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj

#sed -i '' 's|/Users/cagnulein/qdomyos-zwift|..|g' /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj
#add "" for folder qt@5