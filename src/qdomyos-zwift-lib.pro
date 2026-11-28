include(qdomyos-zwift.pri)

TARGET = qdomyos-zwift
TEMPLATE = lib
CONFIG += staticlib
win32:!debug_and_release:CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD/debug
DEFINES += LIB_LIBRARY

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target



