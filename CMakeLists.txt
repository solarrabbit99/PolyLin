cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3")

project(PolyLin VERSION 1.0 LANGUAGES CXX)

set(SOURCE
  "src/engine.cpp"
)

add_executable(engine ${SOURCE})

target_include_directories(engine PRIVATE "include")