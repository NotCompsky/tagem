cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

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
include_directories("${PROJECT_SOURCE_DIR}")

add_subdirectory(manage-tags)
add_subdirectory(media)
add_subdirectory(utils)
if(BUILD_PACKAGES)
    add_subdirectory(packaging)
endif()

#export(TARGETS tagem FILE "${PROJECT_BINARY_DIR}/tagemTargets.cmake")
install(EXPORT tagemTargets DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Registers the build-tree with a global CMake-registry
export(PACKAGE tagem)


# Create the tagemConfig.cmake and tagemConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")

# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(tagemConfig.cmake.in "${PROJECT_BINARY_DIR}/tagemConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${TAGEM_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(tagemConfig.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/tagemConfig.cmake" @ONLY)
# ... for both
configure_file(tagemConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/tagemConfigVersion.cmake" @ONLY)

# Install the tagemConfig.cmake and tagemConfigVersion.cmake
install(
    FILES
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/tagemConfig.cmake"
    "${PROJECT_BINARY_DIR}/tagemConfigVersion.cmake"
    DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev
)

# Install the export set for use with the install-tree
install(EXPORT tagemTargets DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# END src: https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file
