cmake_minimum_required(VERSION 3.10)
project(TrabalhoM1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread")

add_executable(img_sender
    src/sender.cpp
    src/pgm.cpp
)

add_executable(img_worker
    src/worker.cpp
    src/pgm.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(img_sender PRIVATE Threads::Threads)
target_link_libraries(img_worker PRIVATE Threads::Threads)
