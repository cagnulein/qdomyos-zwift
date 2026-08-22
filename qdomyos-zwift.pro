TEMPLATE = subdirs
CONFIG+=ordered

!ios: !android: {
SUBDIRS = \
    src/qdomyos-zwift-lib.pro \
    src/qdomyos-zwift.pro \
    tst/qdomyos-zwift-tests.pro
    
tst.depends = src/qdomyos-zwift-lib.pro
}

android:  {
    SUBDIRS = \
        src/qdomyos-zwift.pro
}

ios: {
    SUBDIRS = \
        src/qdomyos-zwift-lib.pro \
        src/qdomyos-zwift.pro
    
    # Team signing configuration
    QMAKE_IOS_DEPLOYMENT_TARGET = 12.0
    QMAKE_DEVELOPMENT_TEAM = 6335M7T29D
    QMAKE_CODE_SIGN_IDENTITY = "iPhone Developer"
    QMAKE_CODE_SIGN_STYLE = Automatic
    
    # Output directory configuration
    DESTDIR = $$PWD/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug
}

 
