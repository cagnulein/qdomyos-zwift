include(qdomyos-zwift.pri)

# For iOS: Skip actual build and just succeed 
ios {
    message("iOS build: Using manual Xcode project configuration")
    TEMPLATE = aux
    
    # Create a dummy target that always succeeds
    dummy_target.target = dummy
    dummy_target.commands = echo "iOS build completed successfully - using manual Xcode configuration"
    QMAKE_EXTRA_TARGETS += dummy_target
    PRE_TARGETDEPS += dummy
}
