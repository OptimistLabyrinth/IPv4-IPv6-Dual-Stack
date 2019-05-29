cmake_minimum_required(VERSION 3.0)
project(server)

set(CMAKE_CXX_STANDARD 98)

add_executable(server main.cpp)

target_link_libraries(server
    PRIVATE
        pthread
)