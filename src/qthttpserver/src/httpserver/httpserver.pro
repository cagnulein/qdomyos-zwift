TARGET = QtHttpServer
INCLUDEPATH += .

QT = network core-private

qtHaveModule(websockets): QT += websockets-private
qtConfig(ssl): QT += sslserver

HEADERS += \
    qthttpserverglobal.h \
    qabstracthttpserver.h \
    qabstracthttpserver_p.h \
    qhttpserver.h \
    qhttpserver_p.h \
    qhttpserverliterals_p.h \
    qhttpserverrequest.h \
    qhttpserverrequest_p.h \
    qhttpserverresponder.h \
    qhttpserverresponder_p.h \
    qhttpserverresponse.h \
    qhttpserverresponse_p.h \
    qhttpserverrouter.h \
    qhttpserverrouter_p.h \
    qhttpserverrouterrule.h \
    qhttpserverrouterrule_p.h \
    qhttpserverrouterviewtraits.h \
    qhttpserverviewtraits.h \
    qhttpserverviewtraits_impl.h

SOURCES += \
    qabstracthttpserver.cpp \
    qhttpserver.cpp \
    qhttpserverliterals.cpp \
    qhttpserverrequest.cpp \
    qhttpserverresponder.cpp \
    qhttpserverresponse.cpp \
    qhttpserverrouter.cpp \
    qhttpserverrouterrule.cpp

qtHaveModule(concurrent) {
    QT += concurrent
    HEADERS += qhttpserverfutureresponse.h
    SOURCES += qhttpserverfutureresponse.cpp
}

include(../3rdparty/http-parser.pri)

load(qt_module)
