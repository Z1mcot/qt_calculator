cmake_minimum_required(VERSION 3.16)
project(doit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(doit SHARED doit.cpp)
set_target_properties(doit PROPERTIES PREFIX "lib")
