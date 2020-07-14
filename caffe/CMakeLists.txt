cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(OpenCV COMPONENTS core imgcodecs highgui)
find_package(Compsky REQUIRED COMPONENTS asciify mysql)

set(CMAKE_CXX_STANDARD 11)

if(NOT OpenCV_FOUND)
    message(WARNING "OpenCV not found, so tagem-gen-img-db will not be built")
    return()
endif()


add_executable(tagem-gen-img-db src/gen-img-db.cpp)
target_include_directories(tagem-gen-img-db PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(tagem-gen-img-db PRIVATE compsky_asciify compsky_mysql opencv_core opencv_imgcodecs opencv_highgui)


install(
    TARGETS tagem-gen-img-db
    EXPORT tagemTargets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
