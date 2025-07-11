include(qdomyos-zwift.pri)

# For iOS: Skip actual build and just succeed 
ios {    
    # Team signing configuration
    QMAKE_IOS_DEPLOYMENT_TARGET = 12.0
    QMAKE_DEVELOPMENT_TEAM = 6335M7T29D
    QMAKE_CODE_SIGN_IDENTITY = "iPhone Developer"
    QMAKE_CODE_SIGN_STYLE = Automatic
}
