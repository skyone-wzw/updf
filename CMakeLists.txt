cmake_minimum_required(VERSION 3.25)
project(updf)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)
find_package(qpdf CONFIG REQUIRED)

qt_standard_project_setup()

qt_add_executable(updf WIN32
        main.cpp
        mainwindow.hpp
        itemorderlist.hpp
        pdftools.h
        pdftools.cpp)

target_link_libraries(updf PRIVATE
        Qt6::Widgets
        Qt6::Concurrent
        qpdf::libqpdf
)
