# Generated from afterrequest.pro.

cmake_minimum_required(VERSION 3.14)
project(afterrequest LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/httpserver/afterrequest")

find_package(Qt6 COMPONENTS HttpServer)

add_executable(afterrequest
    main.cpp
)
target_link_libraries(afterrequest PUBLIC
    Qt::HttpServer
)

install(TARGETS afterrequest
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
