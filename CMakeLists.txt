cmake_minimum_required(VERSION 2.8.4)
project(CSE4153)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CLIENT_FILES client.cpp)
add_executable(webclient ${CLIENT_FILES})
