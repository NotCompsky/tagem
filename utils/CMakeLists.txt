cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(OpenCV COMPONENTS core imgcodecs highgui)
find_package(Compsky REQUIRED)


set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Type")
option(TAGEM_HASH "Build the tagem-hash utility" ON)
option(CIMG "Compile with CImg (requires libpng, libjpeg, and libtiff)" ON)

set(TGTS)
set(CXX17_TGTS tagem-init)

if(ENABLE_STATIC)
	set(MYSQL_CLIENT_NAMES mariadbclient.a libmariadbclient.a mariadb/mariadbclient.a mariadb/libmariadbclient.a mysqlclient.a libmysqlclient.a mysql/mysqlclient.a mysql/libmysqlclient.a)
	set(OPENCV_CORE_NAMES opencv_core)
	set(OPENCV_ICOD_NAMES opencv_imgcodecs)
	set(OPENCV_HGUI_NAMES opencv_highgui)
	
	set(LIBCRYPTO_NAMES crypto.a libcrypto.a)
	set(LIBSSL_NAMES ssl.a libssl.a openssl.a libopenssl.a)
	set(LIBAVFORMAT_NAMES avformat.a libavformat.a)
	set(LIBAVCODEC_NAMES avcodec.a libavcodec.a)
	
	set(PHASH_NAMES pHash.a libpHash.a phash.a libphash.a)
	
	set(LIBPNG_NAMES png.a libpng.a)
	set(LIBJPEG_NAMES jpeg.a libjpeg.a)
	set(LIBTIFF_NAMES tiff.a libtiff.a)
	set(PTHREAD_NAMES pthread.a libpthread.a)
else()
	set(MYSQL_CLIENT_NAMES mariadbclient mysqlclient)
	set(OPENCV_CORE_NAMES opencv_core.a libopencv_core.a)
	set(OPENCV_ICOD_NAMES opencv_imgcodecs.a libopencv_imgcodecs.a)
	set(OPENCV_HGUI_NAMES opencv_highgui.a libopencv_highgui.a)
	
	set(LIBCRYPTO_NAMES crypto)
	set(LIBSSL_NAMES ssl openssl)
	set(LIBAVFORMAT_NAMES avformat)
	set(LIBAVCODEC_NAMES avcodec)
	
	set(PHASH_NAMES pHash phash)
	
	set(LIBPNG_NAMES png)
	set(LIBJPEG_NAMES jpeg)
	set(LIBTIFF_NAMES tiff)
	set(PTHREAD_NAMES pthread)
endif()
find_library(MYSQL_CLIENT NAMES ${MYSQL_CLIENT_NAMES})
find_library(OPENCV_CORE NAMES ${OPENCV_CORE_NAMES})
find_library(OPENCV_ICOD NAMES ${OPENCV_ICOD_NAMES})
find_library(OPENCV_HGUI NAMES ${OPENCV_HGUI_NAMES})
find_library(LIBSSL NAMES ${LIBSSL_NAMES})
find_library(LIBCRYPTO NAMES ${LIBCRYPTO_NAMES})
find_library(LIBAVFORMAT NAMES ${LIBAVFORMAT_NAMES})
find_library(LIBAVCODEC NAMES ${LIBAVCODEC_NAMES})
find_library(PHASH NAMES ${PHASH_NAMES})

if(CIMG)
	find_library(LIBPNG NAMES ${LIBPNG_NAMES})
	find_library(LIBJPEG NAMES ${LIBJPEG_NAMES})
	find_library(LIBTIFF NAMES ${LIBTIFF_NAMES})
	find_library(PTHREAD NAMES ${PTHREAD_NAMES})
	set(CIMG_DEPS ${LIBPNG} ${LIBJPEG} ${LIBTIFF} ${PTHREAD})
else()
	set(CIMG_DEPS)
endif()

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
	target_link_libraries(tagem-instances PRIVATE "${MYSQL_CLIENT}" "${OPENCV_CORE}" "${OPENCV_ICOD}" "${OPENCV_HGUI}")
    target_compile_definitions(tagem-instances PRIVATE ${MY_DEFINES})
else()
    message(WARNING "OpenCV not found, so tagem-instances will not be built")
endif()

set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")

add_executable(tagem-init src/init.cpp)
target_include_directories(tagem-init PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(tagem-init "${MYSQL_CLIENT}")
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
	target_link_libraries(tagem-hash "${PHASH}" ${CIMG_DEPS} "${MYSQL_CLIENT}" "${LIBSSL}" "${LIBCRYPTO}" "${LIBAVFORMAT}" "${LIBAVCODEC}")
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
