cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(Qt5Widgets 5.9.5 REQUIRED) # Just the version I know works
find_package(Compsky REQUIRED COMPONENTS asciify mysql qt)
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

add_executable(tagem-tags src/main.cpp src/mainwindow.cpp src/primaryitem.cpp src/tagchildtreeview.cpp src/tagparenttreeview.cpp src/tagtreemodel.cpp src/tagtreeview.cpp "${CMAKE_CURRENT_SOURCE_DIR}/../media/src/name_dialog.cpp")
target_compile_definitions(tagem-tags PRIVATE ${MY_DEFINITIONS} QT_GUI_LIB=1)
target_link_libraries(tagem-tags mysqlclient Qt5::Widgets compsky_asciify compsky_mysql)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../media/src")

install(
    TARGETS tagem-tags
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
