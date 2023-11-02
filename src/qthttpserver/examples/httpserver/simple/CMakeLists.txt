# Generated from simple.pro.

cmake_minimum_required(VERSION 3.14)
project(simple LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/httpserver/simple")

find_package(Qt6 COMPONENTS HttpServer)

add_executable(simple
    main.cpp
)
target_link_libraries(simple PUBLIC
    Qt::HttpServer
)


# Resources:
set(assets_resource_files
    "assets/qt-logo.png"
)

qt6_add_resources(simple "assets"
    PREFIX
        "/"
    FILES
        ${assets_resource_files}
)

install(TARGETS simple
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
