brew install qt@5
sed -i '' '/5E618435888B9D49F8540165 \/\* Qt Qmake \*\//d' qdomyoszwift.xcodeproj/project.pbxproj
sed -i '' 's|/Users/cagnulein/qdomyos-zwift|..|g' qdomyoszwift.xcodeproj/project.pbxproj
sed -i '' 's|/Users/cagnulein/Qt/5.15.2/ios/|/usr/local/opt/qt@5/|g' qdomyoszwift.xcodeproj/project.pbxproj
