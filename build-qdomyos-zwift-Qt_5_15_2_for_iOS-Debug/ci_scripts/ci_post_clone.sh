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

cd /Volumes/workspace/repository/src/qthttpserver 
qmake
make -j8
make install

cd /Volumes/workspace/repository/
qmake
make -j8
make install

mkdir /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/QtHttpServer
cp -a /usr/local/Cellar/qt@5/5.15.10_1/lib/QtHttpServer.framework/Versions/5/Headers/*.* /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/QtHttpServer/

ls -ltr /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/QtHttpServer/

git reset -- /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj

#sed -i '' 's|/Users/cagnulein/qdomyos-zwift|..|g' /Volumes/workspace/repository/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj
#add "" for folder qt@5