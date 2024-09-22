cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3")

project(PolyLin VERSION 1.0 LANGUAGES CXX)

# main engine
set(SOURCE
  "src/engine.cpp"
)

add_executable(engine ${SOURCE})

target_include_directories(engine PRIVATE "include")

# priority queue simulator
set(SOURCE
  "src/pqueue_histgen.cpp"
)

find_package(Threads REQUIRED)
add_executable(pq_histgen ${SOURCE})

target_link_libraries(pq_histgen PRIVATE Threads::Threads)
target_include_directories(pq_histgen PRIVATE "include")

# set simulator
set(SOURCE
  "src/set_histgen.cpp"
)

find_package(Threads REQUIRED)
add_executable(set_histgen ${SOURCE})

target_link_libraries(set_histgen PRIVATE Threads::Threads)
target_include_directories(set_histgen PRIVATE "include")