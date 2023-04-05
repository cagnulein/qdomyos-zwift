include(qdomyos-zwift.pri)

TARGET = qdomyoszwift
TEMPLATE = lib
CONFIG += staticlib
DEFINES += LIB_LIBRARY

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

