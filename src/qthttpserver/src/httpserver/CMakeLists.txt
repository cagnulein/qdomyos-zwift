# Generated from httpserver.pro.

#####################################################################
## HttpServer Module:
#####################################################################

qt_add_module(HttpServer
    SOURCES
        ../3rdparty/http-parser/http_parser.c ../3rdparty/http-parser/http_parser.h
        qabstracthttpserver.cpp qabstracthttpserver.h qabstracthttpserver_p.h
        qhttpserver.cpp qhttpserver.h qhttpserver_p.h
        qhttpserverliterals.cpp qhttpserverliterals_p.h
        qhttpserverrequest.cpp qhttpserverrequest.h qhttpserverrequest_p.h
        qhttpserverresponder.cpp qhttpserverresponder.h qhttpserverresponder_p.h
        qhttpserverresponse.cpp qhttpserverresponse.h qhttpserverresponse_p.h
        qhttpserverrouter.cpp qhttpserverrouter.h qhttpserverrouter_p.h
        qhttpserverrouterrule.cpp qhttpserverrouterrule.h qhttpserverrouterrule_p.h
        qhttpserverrouterviewtraits.h
        qhttpserverviewtraits.h
        qhttpserverviewtraits_impl.h
        qthttpserverglobal.h
    INCLUDE_DIRECTORIES
        .
        ../3rdparty/http-parser
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
)

## Scopes:
#####################################################################

qt_extend_target(HttpServer CONDITION TARGET Qt::WebSockets
    LIBRARIES
        Qt::WebSocketsPrivate
    PUBLIC_LIBRARIES
        Qt::WebSockets
    PRIVATE_MODULE_INTERFACE
        Qt::WebSocketsPrivate
)

qt_extend_target(HttpServer CONDITION QT_FEATURE_ssl
    PUBLIC_LIBRARIES
        Qt::SslServer
)

qt_extend_target(HttpServer CONDITION TARGET Qt::Concurrent
    SOURCES
        qhttpserverfutureresponse.cpp qhttpserverfutureresponse.h
    PUBLIC_LIBRARIES
        Qt::Concurrent
)
