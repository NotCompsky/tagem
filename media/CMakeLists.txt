cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
find_package(QtAV REQUIRED)
find_package(Qt5Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)

add_executable(mytagmedia src/main.cpp src/playerwindow.cpp src/playerwindow.h)
target_include_directories(mytagmedia PRIVATE ${QTAVWIDGETS_INCLUDE_DIRS})
target_link_libraries(mytagmedia ${QTAVWIDGETS_LIBRARIES} Qt5::Widgets)
target_link_libraries(mytagmedia mysqlcppconn)

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(mytagmedia PRIVATE DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
    if (MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O3")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    endif()
else()
    message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

# e.g. cmake .. -DCMAKE_BUILD_TYPE=Debug  &&  make clean  &&  VERBOSE=1 make
