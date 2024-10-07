cmake_minimum_required(VERSION 3.10)

project(ServerMMO VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(network)

set(SIMPLE_CLIENT_SRC src/simple_client.cpp)
set(SIMPLE_SERVER_SRC src/simple_server.cpp)

add_executable(simple_client ${SIMPLE_CLIENT_SRC})
add_executable(simple_server ${SIMPLE_SERVER_SRC})
