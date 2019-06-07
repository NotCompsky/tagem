cmake_minimum_required(VERSION 3.14.1 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(Qt5Widgets 5.9.5 REQUIRED) # Just the version I know works
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs highgui)
set(CMAKE_AUTOMOC ON)


set(MY_DEFINES "")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # e.g. cmake .. -DCMAKE_BUILD_TYPE=Debug
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
        set(MY_DEFINES "${MY_DEFINES};DEBUG")
    endif()
else()
    set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")


set(CMAKE_CXX_STANDARD 11)

add_executable(scored src/scored.cpp ../include/utils.cpp ../include/mymysql.cpp ../include/asciify_base.cpp ../include/asciify_core.cpp  ../include/asciify_types.cpp)
target_include_directories(scored PRIVATE "../include")
target_link_libraries(scored PRIVATE mysqlclient)
foreach(def ${MY_DEFINES})
    target_compile_definitions(scored PRIVATE ${def})
endforeach()

add_executable(view-instances src/view-instances.cpp ../include/utils.cpp ../include/mymysql.cpp ../include/asciify_base.cpp ../include/asciify_core.cpp  ../include/asciify_types.cpp)
target_include_directories(view-instances PRIVATE "../include" ${OpenCV_INCLUDE_DIRS})
target_link_libraries(view-instances PRIVATE mysqlclient opencv_core opencv_imgcodecs opencv_highgui)
foreach(def ${MY_DEFINES})
    target_compile_definitions(view-instances PRIVATE ${def})
endforeach()
