cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(Qt5Widgets 5.9.5 REQUIRED) # Just the version I know works
find_package(OpenCV COMPONENTS core imgcodecs highgui)
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

add_executable(tagem-scored src/scored.cpp)
target_link_libraries(tagem-scored PRIVATE mysqlclient compsky_asciify compsky_mysql)
target_compile_definitions(tagem-scored PRIVATE ${MY_DEFINES})

if(OpenCV_FOUND)
    add_executable(tagem-instances src/view-instances.cpp)
    target_include_directories(tagem-instances PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(tagem-instances PRIVATE mysqlclient compsky_asciify compsky_mysql opencv_core opencv_imgcodecs opencv_highgui)
    target_compile_definitions(tagem-instances PRIVATE ${MY_DEFINES})
else()
    message(WARNING "OpenCV not found, so tagem-instances will not be built")
endif()

add_executable(tagem-bulk-tag src/bulk-tag.cpp)
target_link_libraries(tagem-bulk-tag PRIVATE mysqlclient compsky_asciify compsky_mysql)
target_compile_definitions(tagem-bulk-tag PRIVATE ${MY_DEFINES})

add_executable(tagem-srch src/fmt-sql-tags.cpp)
target_link_libraries(tagem-srch PRIVATE mysqlclient compsky_asciify compsky_mysql)
target_compile_definitions(tagem-srch PRIVATE ${MY_DEFINES})

add_executable(tagem-init src/init.cpp)
target_include_directories(tagem-init PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(tagem-init compsky_mysql_create_config compsky_asciify)
set_property(TARGET tagem-init PROPERTY CXX_STANDARD 17)

install(
    TARGETS tagem-scored tagem-instances tagem-bulk-tag tagem-srch tagem-init
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
