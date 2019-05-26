cmake_minimum_required(VERSION 3.14.1 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(QtAV REQUIRED)
find_package(Qt5Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)


if (CMAKE_BUILD_TYPE STREQUAL Debug)
    # e.g. cmake .. -DCMAKE_BUILD_TYPE=Debug  &&  make clean  &&  VERBOSE=1 make
    set(MY_DEFINITIONS "DEBUG")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    endif()
else()
    set(CMAKE_BUILD_TYPE "Release")
    set(MY_DEFINITIONS "QT_NO_DEBUG" "QT_NO_DEBUG_OUTPUT")
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")


set(MY_TARGETS "mytagtxt" "mytagimg" "mytagvid")

foreach(tgt ${MY_TARGETS})
    add_executable("${tgt}" src/main.cpp src/mainwindow.cpp src/mainwindow.h)
    target_include_directories("${tgt}" PRIVATE "../include")
    target_link_libraries("${tgt}" mysqlcppconn)
    target_link_libraries("${tgt}" Qt5::Widgets)
    #target_compile_features("${tgt}" PRIVATE cxx_std_98) # CONTRIBUTIONS WELCOME: Tests
    foreach(dfn ${MY_DEFINITIONS})
        target_compile_definitions("${tgt}" PRIVATE "${dfn}")
    endforeach(dfn)
endforeach(tgt)

# The only difference between these three targets
target_compile_definitions(mytagvid PRIVATE VID BOXABLE)
target_compile_definitions(mytagimg   PRIVATE IMG BOXABLE SCROLLABLE)
target_compile_definitions(mytagtxt   PRIVATE TXT)
target_include_directories(mytagvid PRIVATE ${QTAVWIDGETS_INCLUDE_DIRS})
target_link_libraries(mytagvid ${QTAVWIDGETS_LIBRARIES})
