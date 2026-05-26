cmake_minimum_required(VERSION 3.14)
project(CinemaBookingSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(CinemaBookingSystem
    src/main_console.cpp
)

target_include_directories(CinemaBookingSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data/bilete)
configure_file(data/filme.txt ${CMAKE_CURRENT_BINARY_DIR}/data/filme.txt COPYONLY)
