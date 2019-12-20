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

if(OpenCV_FOUND)
    add_executable(tagem-instances src/view-instances.cpp)
    target_include_directories(tagem-instances PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(tagem-instances PRIVATE compsky_asciify compsky_mysql opencv_core opencv_imgcodecs opencv_highgui)
    target_compile_definitions(tagem-instances PRIVATE ${MY_DEFINES})
else()
    message(WARNING "OpenCV not found, so tagem-instances will not be built")
endif()

add_executable(tagemall src/tagemall.cpp)
target_link_libraries(tagemall PRIVATE compsky_asciify compsky_mysql)
target_compile_definitions(tagemall PRIVATE ${MY_DEFINES})

add_executable(tagem-init src/init.cpp)
target_include_directories(tagem-init PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(tagem-init compsky_mysql_create_config compsky_asciify)
set_property(TARGET tagem-init PROPERTY CXX_STANDARD 17)

if ("${CIMG_H_DIR} " STREQUAL " ")
	message(FATAL_ERROR "Please set CIMG_H_DIR to the directory containing CImg.h (for instance, underneath the third_party of pHash, if you built that from source.")
endif()
set(CMAKE_CXX_STANDARD 17)
add_executable(tagem-hash src/hash.cpp)
target_include_directories(tagem-hash PRIVATE ${COMPSKY_INCLUDE_DIRS} "${CIMG_H_DIR}")
target_link_libraries(tagem-hash pHash compsky_mysql compsky_asciify ssl crypto avformat avcodec boost_regex)

add_executable(tagem-mv src/mv.cpp)
target_include_directories(tagem-mv PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(tagem-mv compsky_mysql compsky_asciify)

find_package(Qt5Widgets REQUIRED)
add_executable(tagem-mk-playlist src/mk_playlist.cpp)
target_include_directories(tagem-mk-playlist PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(tagem-mk-playlist compsky_mysql compsky_asciify Qt5::Widgets)

install(
    TARGETS tagem-instances tagemall tagem-init
    EXPORT tagemTargets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
