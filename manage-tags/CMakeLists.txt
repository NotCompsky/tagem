cmake_minimum_required(VERSION 3.14.1 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(Qt5Widgets 5.9.5 REQUIRED) # Just the version I know works
set(CMAKE_AUTOMOC ON)


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # e.g. cmake .. -DCMAKE_BUILD_TYPE=Debug
    set(MY_DEFINITIONS "DEBUG")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    endif()
else()
    set(CMAKE_BUILD_TYPE "Release")
    set(MY_DEFINITIONS "QT_NO_DEBUG" "QT_NO_DEBUG_OUTPUT")
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")


set(CMAKE_CXX_STANDARD 11)


add_library(primaryitem      src/primaryitem.cpp)
add_library(tagtreemodel     src/tagtreemodel.cpp)
add_library(tagtreeview      src/tagtreeview.cpp)
add_library(mainwindow       src/mainwindow.cpp)
add_library(utils            ../include/utils.cpp)
add_library(asciify          ../include/asciify.cpp)
add_library(mymysql          ../include/mymysql.cpp)
add_library(mymysql_results  ../include/mymysql_results.cpp)
add_library(tagdialog        ../include/tagdialog.cpp)

target_include_directories(primaryitem PRIVATE "../include")
target_include_directories(tagtreeview PRIVATE "../include")
target_include_directories(mainwindow  PRIVATE "../include")

target_link_libraries(tagtreemodel      Qt5::Widgets mymysql_results)
target_link_libraries(primaryitem       Qt5::Widgets mymysql_results tagtreemodel)
target_link_libraries(tagtreeview       Qt5::Widgets mymysql_results tagtreemodel primaryitem)
target_link_libraries(utils             Qt5::Widgets)
target_link_libraries(asciify           Qt5::Widgets)
target_link_libraries(mymysql           Qt5::Widgets mysqlclient utils)
target_link_libraries(mymysql_results   Qt5::Widgets mymysql asciify)
target_link_libraries(tagdialog         Qt5::Widgets)
target_link_libraries(mainwindow        Qt5::Widgets mymysql_results tagtreemodel tagtreeview tagdialog)

add_executable(managetags src/main.cpp)
target_include_directories(managetags PRIVATE "../include")
target_link_libraries(managetags mymysql primaryitem tagtreemodel tagtreeview utils asciify mymysql mymysql_results mainwindow tagdialog) #mainwindow)
