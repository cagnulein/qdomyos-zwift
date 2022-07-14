TEMPLATE = subdirs

QT = network

qtConfig(ssl) {
    SUBDIRS += sslserver
    httpserver.depends = sslserver
}

SUBDIRS += httpserver
