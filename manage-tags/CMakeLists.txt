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

add_executable(managetags src/main.cpp src/primaryitem.cpp src/tagtreemodel.cpp src/tagtreeview.cpp src/mainwindow.cpp ../include/utils.cpp ../include/mymysql.cpp ../include/tagdialog.cpp ../include/asciify_base.cpp ../include/asciify_core.cpp  ../include/asciify_types.cpp)
foreach(def ${MY_DEFINITIONS})
    target_compile_definitions(managetags PRIVATE ${def})
endforeach(def)
target_include_directories(managetags PRIVATE "../include")
target_link_libraries(managetags mysqlclient Qt5::Widgets)
