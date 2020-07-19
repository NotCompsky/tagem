cmake_minimum_required(VERSION 3.7.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

if( EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Refusing to run in-source build.")
endif()

project(tagem CXX) # WARNING: Sets some important variables about the plarform. Don't call find_package before setting a project name.

find_package(Compsky REQUIRED)
include_directories(${COMPSKY_INCLUDE_DIRS})

set(TAGEM_MAJOR_VERSION 0)
set(TAGEM_MINOR_VERSION 1)
set(TAGEM_PATCH_VERSION 0)
set(TAGEM_VERSION ${TAGEM_MAJOR_VERSION}.${TAGEM_MINOR_VERSION}.${TAGEM_PATCH_VERSION})




# BEGIN src: https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file

if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/tagem)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

# set up include-directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}")

if(EXISTS utils)
	add_subdirectory(utils)
endif()
if(EXISTS caffe)
	add_subdirectory(caffe)
endif()

# END src: https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file
