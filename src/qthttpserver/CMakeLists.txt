# Generated from qthttpserver.pro.

cmake_minimum_required(VERSION 3.15.0)

project(QtHttpServer
    VERSION 6.0.0
    DESCRIPTION "Qt HTTP Server"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Network)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Concurrent)

qt_build_repo()
