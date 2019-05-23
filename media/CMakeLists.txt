cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
find_package(QtAV REQUIRED)
find_package(Qt5Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)

add_executable(mytagmedia src/main.cpp src/playerwindow.cpp src/playerwindow.h)
target_include_directories(mytagmedia PRIVATE ${QTAVWIDGETS_INCLUDE_DIRS})
target_include_directories(mytagmedia PRIVATE "../include")
target_link_libraries(mytagmedia ${QTAVWIDGETS_LIBRARIES} Qt5::Widgets)
target_link_libraries(mytagmedia mysqlcppconn)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(mytagmedia PRIVATE DEBUG)
else(CMAKE_BUILD_TYPE STREQUAL Release)
    set(CMAKE_BUILD_TYPE "Release")
    target_compile_definitions(mytagmedia PRIVATE QT_NO_DEBUG_OUTPUT)
    target_compile_definitions(mytagmedia PRIVATE QT_NO_DEBUG)
    target_compile_definitions(mytagmedia PRIVATE O3)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# e.g. cmake .. -DCMAKE_BUILD_TYPE=Debug  &&  make clean  &&  VERBOSE=1 make
