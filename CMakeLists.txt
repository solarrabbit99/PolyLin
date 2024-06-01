cmake_minimum_required(VERSION 3.16)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_FLAGS "-O3")

project(PolyLin VERSION 1.0 LANGUAGES CXX)

file(GLOB FILES CONFIGURE_DEPENDS "src/*.hpp" "src/*.cpp")

add_executable(engine ${FILES})