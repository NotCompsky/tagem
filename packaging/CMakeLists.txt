#Copyright (c) 2019 Kracejic

#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


# Original file here: https://github.com/kracejic/cleanCppProject/blob/ba579d8fdbcc880cff45cb0b1b5ac165bf8727a8/packaging/CMakeLists.txt
# Modified to fit tagem package specifics, and will likely continue to evolve/diverge.


#------------------------------------------------------------------------------
# CPACK
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_VENDOR "NotCompsky")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT "NotCompsky <adammarkgray@gmail.com>")
set(HOMEPAGE "https://github.com/NotCompsky/tagem")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Family of resources written in C++ for batch tagging files, including preparing images for recognition model generation.")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR ${TAGEM_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${TAGEM_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${TAGEM_PATCH_VERSION})
set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})

set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/pkg")

#set(CPACK_PACKAGE_FILE_NAME DEB-DEFAULT) # Incompatible with default CMake version on Ubuntu 18.04
if("${CPACK_DEBIAN_PACKAGE_ARCHITECTURE} " STREQUAL " ")
    message(FATAL_ERROR "Please set -DCPACK_DEBIAN_PACKAGE_ARCHITECTURE=$(dpkg --print-architecture)")
endif()
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")


set(CPACK_CREATE_DESKTOP_LINKS tagem-tags)
set(CPACK_STRIP_FILES TRUE)

if(APPLE OR NOT UNIX)
    message(FATAL_ERROR "I haven't written the CMake to package on non-Linux systems yet")
else()
    #-----------------------------------------------------------------------------
    # Linux specific
    set(CPACK_GENERATOR "DEB;TBZ2;TXZ")
    message(STATUS "Package generation - UNIX")
    message(STATUS "   + DEB                                  YES ")
    message(STATUS "   + TBZ2                                 YES ")
    message(STATUS "   + TXZ                                  YES ")

    find_program(RPMBUILD_PATH rpmbuild)
    if(RPMBUILD_PATH)
        message(STATUS "   + RPM                                  YES ")
        set(CPACK_GENERATOR "${CPACK_GENERATOR};RPM")
        set(CPACK_RPM_PACKAGE_LICENSE "MIT")
        # set(CPACK_RPM_PACKAGE_REQUIRES "gtkmm30")
        # exclude folders which clash with default ones
        set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST
            ${CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST}
            /usr
            /usr/bin
            /usr/share
            /usr/share/applications
            /usr/share/doc
            /usr/share/icons
            /usr/share/icons/hicolor
            /usr/share/icons/hicolor/256x256
            /usr/share/icons/hicolor/256x256/apps
            /usr/share/icons/gnome
            /usr/share/icons/gnome/256x256
            /usr/share/icons/gnome/256x256/apps)
    else()
        message(STATUS "   + RPM                                  NO ")
    endif()

    set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${HOMEPAGE}")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcompsky, libqt5widgets5")
endif()




#------------------------------------------------------------------------------
# and include CPack, so we get target for packages
include(CPack)
