cmake_minimum_required(VERSION 2.8)
project(qabstracthttpserver_test)

find_package(Qt5 COMPONENTS HttpServer REQUIRED)

add_executable(qabstracthttpserver_test "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_link_libraries(qabstracthttpserver_test Qt5::HttpServer)
