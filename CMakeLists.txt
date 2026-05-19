cmake_minimum_required(VERSION 3.16)
project(CinemaBookingSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

qt_standard_project_setup()

qt_add_executable(CinemaBookingSystem
    src/main.cpp
    src/mainwindow_builders.cpp
    src/mainwindow_slots.cpp
    include/mainwindow.h
    include/Cinema.h
    include/CinemaModels.h
    include/config.h
)

target_include_directories(CinemaBookingSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(CinemaBookingSystem PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

foreach(datafile filme.txt toate_rezervarile.txt istoric_Diana.txt)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data/${datafile}")
        configure_file(data/${datafile} ${CMAKE_CURRENT_BINARY_DIR}/${datafile} COPYONLY)
    endif()
endforeach()

if(WIN32)
    set_target_properties(CinemaBookingSystem PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
