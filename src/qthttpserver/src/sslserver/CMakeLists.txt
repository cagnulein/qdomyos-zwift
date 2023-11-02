# Generated from sslserver.pro.

#####################################################################
## SslServer Module:
#####################################################################

qt_add_module(SslServer
    SOURCES
        qsslserver.cpp qsslserver.h qsslserver_p.h
        qtsslserverglobal.h
    INCLUDE_DIRECTORIES
        .
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
)
