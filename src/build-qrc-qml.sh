/Users/cagnulein/Qt/5.15.2/ios/bin/rcc qml.qrc -o ../build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qrc_qml.cpp
# the settings pages compiled at build time, as qdomyos-zwift.pri makes them: one line per page of
# settings-qmlcache.qrc, then the loader. The Xcode project compiles these three files and the app
# shows the version of the page they were made from, so they are made again together with qrc_qml.cpp
/Users/cagnulein/Qt/5.15.2/ios/bin/qmlcachegen --resource=settings-qmlcache.qrc -o ../build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qmlcache_settings_qml.cpp settings.qml
/Users/cagnulein/Qt/5.15.2/ios/bin/qmlcachegen --resource=settings-qmlcache.qrc -o ../build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qmlcache_settings-tiles_qml.cpp settings-tiles.qml
/Users/cagnulein/Qt/5.15.2/ios/bin/qmlcachegen --resource-file-mapping=settings-qmlcache.qrc -o ../build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/settings_qmlcache_loader.cpp settings-qmlcache.qrc
