cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(OpenCV COMPONENTS core imgcodecs highgui)
find_package(Compsky REQUIRED)


set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Type")

if(OpenCV_FOUND)
    add_executable(tagem-instances src/view-instances.cpp)
	set_target_properties(
		tagem-instances
		PROPERTIES
			CXX_STANDARD 11
			INTERPROCEDURAL_OPTIMIZATION TRUE
			LINK_FLAGS_RELEASE -s
	)
    target_include_directories(tagem-instances PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(tagem-instances PRIVATE compsky_mysql opencv_core opencv_imgcodecs opencv_highgui)
    target_compile_definitions(tagem-instances PRIVATE ${MY_DEFINES})
    
	install(
		TARGETS tagem-instances
		EXPORT tagemTargets
		RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
			COMPONENT bin
	)
else()
    message(WARNING "OpenCV not found, so tagem-instances will not be built")
endif()

add_executable(tagem-init src/init.cpp)
target_include_directories(tagem-init PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(tagem-init compsky_mysql_create_config)
set_property(TARGET tagem-init PROPERTY CXX_STANDARD 17)

if ("${CIMG_H_DIR} " STREQUAL " ")
	find_package(CImg REQUIRED)
	set(CIMG_H_DIR "${CImg_INCLUDE_DIR}")
	message(FATAL_ERROR "Please set CIMG_H_DIR to the directory containing CImg.h (for instance, underneath the third_party of pHash, if you built that from source.")
endif()
add_executable(tagem-hash src/hash.cpp)
target_include_directories(tagem-hash PRIVATE ${COMPSKY_INCLUDE_DIRS} "${CIMG_H_DIR}")
target_link_libraries(tagem-hash pHash compsky_mysql ssl crypto avformat avcodec boost_regex)

add_executable(tagem-mk-playlist src/mk_playlist.cpp)
target_include_directories(tagem-mk-playlist PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(tagem-mk-playlist compsky_mysql avformat avcodec avutil)

set_target_properties(
	tagem-hash tagem-init
	PROPERTIES
		CXX_STANDARD 17
		INTERPROCEDURAL_OPTIMIZATION TRUE
		LINK_FLAGS_RELEASE -s
)

install(
    TARGETS tagem-hash tagem-init
    EXPORT tagemTargets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
