TARGET = QtSslServer
INCLUDEPATH += .

QT = network core

HEADERS += \
    qsslserver.h \
    qtsslserverglobal.h \
    qsslserver_p.h

SOURCES += \
    qsslserver.cpp

load(qt_module)
