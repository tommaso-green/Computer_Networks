cmake_minimum_required(VERSION 3.10)
project(Computer_Networks C)

set(CMAKE_C_STANDARD 99)

add_executable(HTTP1.0 wc20.10_explained.c)
add_executable(HTTP1.1 wc20.11_explained.c)
add_executable(Web_Server ws20_explained.c)
add_executable(Web_Proxy wp20_explained.c)
add_executable(Ping ping20_explained.c)
add_executable(Printing printing_functions.c)