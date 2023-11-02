# Generated from qhttpserver.pro.

#####################################################################
## tst_qhttpserver Test:
#####################################################################

# Collect test data
list(APPEND test_data "data/")

qt_add_test(tst_qhttpserver
    SOURCES
        tst_qhttpserver.cpp
    PUBLIC_LIBRARIES
        Qt::HttpServer
        Qt::HttpServerPrivate
    TESTDATA ${test_data}
)
