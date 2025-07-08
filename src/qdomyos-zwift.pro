include(qdomyos-zwift.pri)

# For iOS: Use existing Xcode project instead of regenerating
ios {
    # Don't regenerate Xcode project if it exists with our manual modifications
    exists($$OUT_PWD/qdomyoszwift.xcodeproj/project.pbxproj) {
        message("Using existing Xcode project with manual code signing configuration")
        # Change to a template that doesn't regenerate Xcode project
        TEMPLATE = aux
        CONFIG -= app_bundle
        # Just link to existing app
        QMAKE_LINK = echo "Using existing Xcode project"
        QMAKE_LINK_C = echo "Using existing Xcode project"  
        QMAKE_LINK_C_SHLIB = echo "Using existing Xcode project"
    }
}
