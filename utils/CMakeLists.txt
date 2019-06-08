cmake_minimum_required(VERSION 3.14.1 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(Qt5Widgets 5.9.5 REQUIRED) # Just the version I know works
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs highgui)
find_package(Compsky REQUIRED COMPONENTS asciify mysql)
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

add_executable(scored src/scored.cpp)
target_include_directories(scored PRIVATE "../include")
target_link_libraries(scored PRIVATE mysqlclient compsky_asciify compsky_mysql)
foreach(def ${MY_DEFINES})
    target_compile_definitions(scored PRIVATE ${def})
endforeach()

add_executable(view-instances src/view-instances.cpp)
target_include_directories(view-instances PRIVATE "../include" ${OpenCV_INCLUDE_DIRS})
target_link_libraries(view-instances PRIVATE mysqlclient compsky_asciify compsky_mysql opencv_core opencv_imgcodecs opencv_highgui)
foreach(def ${MY_DEFINES})
    target_compile_definitions(view-instances PRIVATE ${def})
endforeach()

add_executable(bulk-tag src/bulk-tag.cpp)
target_include_directories(bulk-tag PRIVATE "../include" ${OpenCV_INCLUDE_DIRS})
target_link_libraries(bulk-tag PRIVATE mysqlclient compsky_asciify compsky_mysql)
foreach(def ${MY_DEFINES})
    target_compile_definitions(bulk-tag PRIVATE ${def})
endforeach()

add_executable(fmt-sql-tags src/fmt-sql-tags.cpp)
target_include_directories(fmt-sql-tags PRIVATE "../include" ${OpenCV_INCLUDE_DIRS})
target_link_libraries(fmt-sql-tags PRIVATE mysqlclient compsky_asciify compsky_mysql)
foreach(def ${MY_DEFINES})
    target_compile_definitions(fmt-sql-tags PRIVATE ${def})
endforeach()
