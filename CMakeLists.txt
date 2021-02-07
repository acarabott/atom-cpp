cmake_minimum_required(VERSION 3.10)

set(TARGET "atom-cpp")

project(${TARGET} VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED  True)

add_executable(${TARGET} ./src/Main.cpp)
