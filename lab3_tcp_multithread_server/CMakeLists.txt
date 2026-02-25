cmake_minimum_required(VERSION 3.18)
project(udp_server)

set(SOURCE_DIR src)

add_executable(server ${SOURCE_DIR}/server.cpp)
add_executable(client ${SOURCE_DIR}/client.cpp)