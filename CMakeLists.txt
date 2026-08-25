cmake_minimum_required(VERSION 4.3)
project(updf)

set(CMAKE_CXX_STANDARD 23)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(QPDF REQUIRED IMPORTED_TARGET libqpdf)

qt_standard_project_setup()

qt_add_executable(updf
        main.cpp
        mainwindow.hpp
        itemorderlist.hpp
        pdfmergeworker.hpp)

target_link_libraries(updf PRIVATE Qt6::Widgets PkgConfig::QPDF)
