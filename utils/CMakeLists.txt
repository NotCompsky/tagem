cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(OpenCV COMPONENTS core imgcodecs highgui)
find_package(Compsky REQUIRED)


set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Type")
option(TAGEM_HASH "Build the tagem-hash utility" ON)

set(TGTS)
set(CXX17_TGTS tagem-init)

if(ENABLE_STATIC)
	set(MYSQL_CLIENT_NAMES mariadbclient.a libmariadbclient.a mariadb/mariadbclient.a mariadb/libmariadbclient.a mysqlclient.a libmysqlclient.a mysql/mysqlclient.a mysql/libmysqlclient.a)
else()
	set(MYSQL_CLIENT_NAMES mariadbclient mysqlclient)
endif()
find_library(MYSQL_CLIENT NAMES ${MYSQL_CLIENT_NAMES})

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
    target_link_libraries(tagem-instances PRIVATE "${MYSQL_CLIENT}" opencv_core opencv_imgcodecs opencv_highgui)
    target_compile_definitions(tagem-instances PRIVATE ${MY_DEFINES})
else()
    message(WARNING "OpenCV not found, so tagem-instances will not be built")
endif()

set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")

add_executable(tagem-init src/init.cpp)
target_include_directories(tagem-init PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(tagem-init compsky_mysql_create_config "${MYSQL_CLIENT}")
set_property(TARGET tagem-init PROPERTY CXX_STANDARD 17)

if(TAGEM_HASH)
	if(NOT PHASH_INCLUDE_DIR)
		find_path(PHASH_INCLUDE_DIR NAMES pHash.h HINTS /usr/include /usr/local/include /usr)
		if(NOT PHASH_INCLUDE_DIR)
			message(FATAL_ERROR "Cannot find pHash.h - please either install pHash or set PHASH_INCLUDE_DIR to a directory including pHash.h")
		endif()
	endif()
	set(CXX17_TGTS "${CXX17_TGTS};tagem-hash")
	set(TGTS "${TGTS};tagem-hash")
	if (NOT CIMG_H_DIR)
		find_path(CIMG_H_DIR NAMES CImg.h HINTS /usr/include /usr/local/include /usr)
		if(NOT CIMG_H_DIR)
			message(FATAL_ERROR "Please set CIMG_H_DIR to the directory containing CImg.h (for instance, underneath the third_party of pHash, if you built that from source.")
		endif()
	endif()
	add_executable(tagem-hash src/hash.cpp)
	target_include_directories(tagem-hash PRIVATE "${PROJECT_INCLUDE_DIR}" ${COMPSKY_INCLUDE_DIRS} "${CIMG_H_DIR}" "${PHASH_INCLUDE_DIR}")
	target_link_directories(tagem-hash PRIVATE "${PHASH_INCLUDE_DIR}")
	target_link_libraries(tagem-hash pHash "${MYSQL_CLIENT}" ssl crypto avformat avcodec boost_regex)
endif()

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
