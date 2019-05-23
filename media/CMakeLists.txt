cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
find_package(QtAV REQUIRED)
find_package(Qt5Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)

add_executable(mytagmedia src/main.cpp src/mainwindow.cpp src/mainwindow.h)
add_executable(mytagtxt   src/main.cpp src/mainwindow.cpp src/mainwindow.h src/customplaintextedit.cpp src/customplaintextedit.h)

# The only difference
target_compile_definitions(mytagmedia PRIVATE _FILE_TYPE_=0)
target_compile_definitions(mytagtxt   PRIVATE _FILE_TYPE_=1)

target_include_directories(mytagmedia PRIVATE ${QTAVWIDGETS_INCLUDE_DIRS})
target_include_directories(mytagtxt   PRIVATE ${QTAVWIDGETS_INCLUDE_DIRS})
target_include_directories(mytagmedia PRIVATE "../include")
target_include_directories(mytagtxt   PRIVATE "../include")
target_link_libraries(mytagmedia ${QTAVWIDGETS_LIBRARIES} Qt5::Widgets)
target_link_libraries(mytagtxt   ${QTAVWIDGETS_LIBRARIES} Qt5::Widgets)
target_link_libraries(mytagmedia mysqlcppconn)
target_link_libraries(mytagtxt   mysqlcppconn)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(mytagmedia PRIVATE DEBUG)
    target_compile_definitions(mytagtxt   PRIVATE DEBUG)
else(CMAKE_BUILD_TYPE STREQUAL Release)
    set(CMAKE_BUILD_TYPE "Release")
    target_compile_definitions(mytagmedia PRIVATE QT_NO_DEBUG_OUTPUT)
    target_compile_definitions(mytagtxt   PRIVATE QT_NO_DEBUG_OUTPUT)
    target_compile_definitions(mytagmedia PRIVATE QT_NO_DEBUG)
    target_compile_definitions(mytagtxt   PRIVATE QT_NO_DEBUG)
    target_compile_definitions(mytagmedia PRIVATE O3)
    target_compile_definitions(mytagtxt   PRIVATE O3)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# e.g. cmake .. -DCMAKE_BUILD_TYPE=Debug  &&  make clean  &&  VERBOSE=1 make
