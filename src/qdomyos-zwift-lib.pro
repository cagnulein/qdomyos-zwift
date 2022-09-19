include(qdomyos-zwift.pri)

TARGET = qdomyos-zwift
TEMPLATE = lib
CONFIG += staticlib
DEFINES += LIB_LIBRARY

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
S
