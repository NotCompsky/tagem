cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(QtAV REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Compsky REQUIRED COMPONENTS asciify mysql)
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


if ("${MY_TARGETS} " STREQUAL " ") # Suffix character is deliberate; simply does not match empty strings without it
    # e.g. cmake .. -DMY_TARGETS=txt;img
    set(MY_TARGETS "txt" "img" "vid")
endif()
message(STATUS "Targets: ${MY_TARGETS}")

foreach(file_type ${MY_TARGETS})
    set(tgt "tagem${file_type}")
    add_executable("${tgt}"
		src/main.cpp
		src/mainwindow.cpp
		src/keyreceiver.cpp
		src/name_dialog.cpp
		src/inlist_filter_dialog.cpp
		src/dropdown_dialog.cpp
		src/info_dialog.cpp
		src/unlink_tag_btn.cpp
		src/instance_relation_dialog.cpp
		
		src/relation_add_instance_tags.cpp
		src/relation_add_instance_tags_rule.cpp
	)
    target_include_directories("${tgt}" PRIVATE "../include")
    target_link_libraries("${tgt}" Qt5::Widgets compsky_asciify compsky_mysql)
    foreach(dfn ${MY_DEFINITIONS})
        target_compile_definitions("${tgt}" PRIVATE "${dfn}")
    endforeach(dfn)
endforeach(file_type)


# The only difference between these three targets
if ("txt" IN_LIST MY_TARGETS)
    target_compile_definitions(tagemtxt PRIVATE TXT)
endif()
if ("img" IN_LIST MY_TARGETS)
    target_compile_definitions(tagemimg PRIVATE IMG BOXABLE SCROLLABLE)
    target_sources(tagemimg PRIVATE src/overlay.cpp src/instancewidgetbutton.cpp src/instancewidget.cpp src/instancerelation.cpp)
endif()
if ("vid" IN_LIST MY_TARGETS)
    target_compile_definitions(tagemvid PRIVATE VID AUDIO BOXABLE)
    target_include_directories(tagemvid PRIVATE ${QTAVWIDGETS_INCLUDE_DIRS})
    target_link_libraries(tagemvid ${QTAVWIDGETS_LIBRARIES})
    target_sources(tagemvid PRIVATE src/overlay.cpp src/instancewidgetbutton.cpp src/instancewidget.cpp src/instancerelation.cpp)
endif()


if(IS_DIRECTORY "${CMAKE_INSTALL_PREFIX}/man/man1")
    # Manpage creation strongly based on example by Michael Wild in CMake mailing list in September 2010
    # https://cmake.org/pipermail/cmake/2010-September/039781.html
    set(MAN_PAGES_BASENAMES "tagem.1" "tagemtxt.1" "tagemvid.1")
    set(DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/docs")
    set(MAN_FILES)
    find_program(PANDOC NAMES pandoc)
    foreach(basename ${MAN_PAGES_BASENAMES})
        set(tgt "${basename}")
        add_custom_command(
            COMMAND "${PANDOC}" -s -t man "${DOC_DIR}/${basename}.md" -o "${CMAKE_CURRENT_BINARY_DIR}/${basename}"
            COMMENT "Generating manpage from markdown doc: ${basename}"
            DEPENDS "${DOC_DIR}/${basename}.md"
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${basename}"
        )
        list(APPEND MAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/${basename}")
    endforeach()

    add_custom_target(man ALL DEPENDS ${MAN_FILES})
    INSTALL(
        FILES ${MAN_FILES}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man1"
    )
endif()


install(
    TARGETS tagemtxt tagemimg tagemvid
    EXPORT tagemTargets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
