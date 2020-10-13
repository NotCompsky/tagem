cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(OpenCV COMPONENTS core imgcodecs highgui)
find_package(Compsky REQUIRED)


set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Type")
option(TAGEM_HASH "Build the tagem-hash utility" OFF)
set(WHICH_MYSQL_CLIENT "mysqlclient" CACHE STRING "Which MySQL or MariaDB client to use")

set(TGTS)
set(CXX17_TGTS tagem-init)

if(OpenCV_FOUND)
	set(TGTS "${TGTS};tagem-instances")
    add_executable(tagem-instances src/view-instances.cpp)
	set_target_properties(
		tagem-instances
		PROPERTIES
			CXX_STANDARD 11
			INTERPROCEDURAL_OPTIMIZATION TRUE
			LINK_FLAGS_RELEASE -s
	)
    target_include_directories(tagem-instances PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(tagem-instances PRIVATE "${WHICH_MYSQL_CLIENT}" opencv_core opencv_imgcodecs opencv_highgui)
    target_compile_definitions(tagem-instances PRIVATE ${MY_DEFINES})
else()
    message(WARNING "OpenCV not found, so tagem-instances will not be built")
endif()

set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")

add_executable(tagem-init src/init.cpp)
target_include_directories(tagem-init PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(tagem-init compsky_mysql_create_config "${WHICH_MYSQL_CLIENT}")
set_property(TARGET tagem-init PROPERTY CXX_STANDARD 17)

if(TAGEM_HASH)
	list(CXX17_TGTS APPEND tagem-hash)
	list(TGTS APPEND tagem-hash)
	if ("${CIMG_H_DIR} " STREQUAL " ")
		find_package(CImg REQUIRED)
		set(CIMG_H_DIR "${CImg_INCLUDE_DIR}")
		message(FATAL_ERROR "Please set CIMG_H_DIR to the directory containing CImg.h (for instance, underneath the third_party of pHash, if you built that from source.")
	endif()
	add_executable(tagem-hash src/hash.cpp)
	target_include_directories(tagem-hash PRIVATE "${PROJECT_INCLUDE_DIR}" ${COMPSKY_INCLUDE_DIRS} "${CIMG_H_DIR}")
	target_link_libraries(tagem-hash pHash compsky_mysql ssl crypto avformat avcodec boost_regex)
endif()

#add_executable(tagem-mk-playlist src/mk_playlist.cpp)
#target_include_directories(tagem-mk-playlist PRIVATE ${COMPSKY_INCLUDE_DIRS})
#target_link_libraries(tagem-mk-playlist compsky_mysql avformat avcodec avutil)

set_target_properties(
	${CXX17_TGTS}
	PROPERTIES
		CXX_STANDARD 17
		INTERPROCEDURAL_OPTIMIZATION TRUE
		LINK_FLAGS_RELEASE -s
)

install(
    TARGETS ${TGTS}
    EXPORT tagemTargets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
