cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(QtAV REQUIRED)
find_package(Qt5Widgets REQUIRED)
if (LOCAL_ONLY)
	set(MY_DEFINITIONS "${MY_DEFINITIONS}" "LOCAL_ONLY")
	message(WARNING "LOCAL_ONLY restriction is not properly implemented yet")
else()
	find_package(Qt5Network REQUIRED)
endif()
find_package(Compsky REQUIRED COMPONENTS asciify mysql)
set(CMAKE_AUTOMOC ON)


set(CMAKE_CXX_STANDARD 17)


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # e.g. cmake .. -DCMAKE_BUILD_TYPE=Debug
    set(MY_DEFINITIONS "${MY_DEFINITIONS};DEBUG")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    endif()
else()
    set(CMAKE_BUILD_TYPE "Release")
    set(MY_DEFINITIONS "${MY_DEFINITIONS}" "QT_NO_DEBUG" "QT_NO_DEBUG_OUTPUT")
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")


if ("${MY_TARGETS} " STREQUAL " ") # Suffix character is deliberate; simply does not match empty strings without it
    # e.g. cmake .. -DMY_TARGETS=txt;img
    set(MY_TARGETS "txt" "img" "vid")
endif()
message(STATUS "Targets: ${MY_TARGETS}")

set(BOXABLE_SOURCES
	src/boxes/box_widget_btn.cpp
	src/boxes/box_widget.cpp
	src/boxes/box_relation.cpp
	src/boxes/box_relation_dialog.cpp
	
	src/relation-manager/relation_add_box_tags.cpp
	src/relation-manager/relation_add_box_tags_rule.cpp
)
set(ERA_SOURCES
	src/era.cpp
	src/era-manager/era_manager.cpp
)
set(OVERLAY_SOURCES
	src/overlay.cpp
)
set(SUBTITLES_SOURCES
	src/textbox.cpp
)
set(MENU_SOURCES
	src/menu.cpp
)
set(COMPLETER_SOURCES
	src/completer/simplify_str.cpp
)
set(TAG_MANAGER_SOURCES
	src/tag-manager/mainwindow.cpp
	src/tag-manager/primaryitem.cpp
	src/tag-manager/tagchildtreeview.cpp
	src/tag-manager/tagparenttreeview.cpp
	src/tag-manager/tagtreemodel.cpp
	src/tag-manager/tagtreeview.cpp
)
set(NETWORK_SOURCES
	src/get_from_network.cpp
)

add_executable(tagem-tag-manager
	src/main.cpp
	
	src/add_new_tag.cpp
	src/name_dialog.cpp
	src/dropdown_dialog.cpp
	src/unlink_tag_btn.cpp
	src/add_new_tag.cpp
)
target_compile_definitions(tagem-tag-manager PRIVATE TAG_MANAGER)
target_include_directories(tagem-tag-manager PRIVATE "../include")
target_link_libraries(tagem-tag-manager Qt5::Widgets compsky_asciify compsky_mysql)
target_sources(tagem-tag-manager PRIVATE ${TAG_MANAGER_SOURCES})
target_sources(tagem-tag-manager PRIVATE ${COMPLETER_SOURCES})

add_executable(tagem-add-tag
	src/main.cpp
	src/name_dialog.cpp
	src/add_new_tag.cpp
)
target_include_directories(tagem-add-tag PRIVATE "../include")
target_link_libraries(tagem-add-tag Qt5::Widgets compsky_asciify compsky_mysql)
target_sources(tagem-add-tag PRIVATE ${COMPLETER_SOURCES})

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
		src/add_new_tag.cpp
		
		src/file2.cpp
	)
	if(NOT LOCAL_ONLY)
		if("${tgt}" STREQUAL tagemimg  OR  "${tgt}" STREQUAL tagemtxt)
			target_sources("${tgt}" PRIVATE ${NETWORK_SOURCES})
		endif()
	endif()
	target_sources("${tgt}" PRIVATE ${TAG_MANAGER_SOURCES})
	target_compile_definitions("${tgt}" PRIVATE MAINWINDOW FILE2)
    target_include_directories("${tgt}" PRIVATE "../include")
    target_link_libraries("${tgt}" Qt5::Widgets Qt5::Network compsky_asciify compsky_mysql)
    foreach(dfn ${MY_DEFINITIONS})
        target_compile_definitions("${tgt}" PRIVATE "${dfn}")
    endforeach(dfn)
	if(NOT "${PYTHON_DIR} " STREQUAL " ")
		target_include_directories("${tgt}" PRIVATE "${PYTHON_DIR}")
		target_link_libraries("${tgt}" "${PYTHON}")
		target_compile_definitions("${tgt}" PRIVATE PYTHON)
	endif()
	target_sources("${tgt}" PRIVATE ${COMPLETER_SOURCES})
endforeach(file_type)



add_executable(cli src/cli.cpp)
target_link_libraries(cli Qt5::Widgets compsky_asciify compsky_mysql)



# The only difference between these three targets
if ("txt" IN_LIST MY_TARGETS)
    target_compile_definitions(tagemtxt PRIVATE TXT)
endif()
if ("img" IN_LIST MY_TARGETS)
    target_compile_definitions(tagemimg PRIVATE IMG BOXABLE SCROLLABLE OVERLAY)
    target_sources(tagemimg PRIVATE ${BOXABLE_SOURCES})
    target_sources(tagemimg PRIVATE ${OVERLAY_SOURCES})
endif()
if ("vid" IN_LIST MY_TARGETS)
    target_compile_definitions(tagemvid PRIVATE VID AUDIO BOXABLE ERA OVERLAY SUBTITLES)
	if (NOT "${PYTHON} " STREQUAL " ")
		target_compile_definitions(tagemvid PRIVATE MENUS)
		target_sources(tagemvid PRIVATE ${MENU_SOURCES})
	endif()
    target_sources(tagemvid PRIVATE ${BOXABLE_SOURCES})
    target_sources(tagemvid PRIVATE ${ERA_SOURCES})
    target_sources(tagemvid PRIVATE ${OVERLAY_SOURCES})
	target_sources(tagemvid PRIVATE ${SUBTITLES_SOURCES})
    target_include_directories(tagemvid PRIVATE ${QTAVWIDGETS_INCLUDE_DIRS})
    target_link_libraries(tagemvid ${QTAVWIDGETS_LIBRARIES})
endif()


if(IS_DIRECTORY "${CMAKE_INSTALL_PREFIX}/man/man1")
    # Manpage creation strongly based on example by Michael Wild in CMake mailing list in September 2010
    # https://cmake.org/pipermail/cmake/2010-September/039781.html
    set(MAN_PAGES_BASENAMES "tagem.1" "tagemtxt.1" "tagemvid.1")
    set(DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/docs")
    set(MAN_FILES)
    find_program(PANDOC NAMES pandoc)
	if ("${PANDOC-NOTFOUND}")
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
	endif()

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
