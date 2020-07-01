cmake_minimum_required(VERSION 3.13)

# If you have installed Proxygen, you can use
# cmake -Dproxygen_root_DIR=/path/to/proxygen/root/dir

if(proxygen_root_DIR)
	set(proxygen_cmake_DIR "${proxygen_root_DIR}/proxygen/_build/deps/lib/cmake")
	
	if(NOT wangle_DIR)
		set(wangle_DIR "${proxygen_cmake_DIR}/wangle")
	endif()
	
	if(NOT folly_DIR)
		set(folly_DIR "${proxygen_cmake_DIR}/folly")
	endif()
	
	if(NOT fizz_DIR)
		set(fizz_DIR "${proxygen_cmake_DIR}/fizz")
	endif()
else()
	message(WARNING "You might want to define -Dproxygen_root_DIR=/path/to/proxygen/root for ease - defines proxygen_cmake_DIR, wangle_DIR, folly_DIR and fizz_DIR")
endif()

find_package(fizz REQUIRED)
find_package(Threads REQUIRED)
find_package(folly REQUIRED)
find_package(wangle REQUIRED)
find_package(gflags REQUIRED)
find_package(Compsky REQUIRED)
find_package(CURL REQUIRED)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")

add_executable(qry "${SRC_DIR}/qry-cli.cpp" "${SRC_DIR}/qry.cpp")
set_property(TARGET qry PROPERTY CXX_STANDARD 17)
target_include_directories(qry PRIVATE "${COMPSKY_INCLUDE_DIRS}")
target_compile_definitions(qry PRIVATE DEBUG)

add_executable(server "${SRC_DIR}/server.cpp" "${SRC_DIR}/FrameDecoder.cpp" "${SRC_DIR}/qry.cpp" "${SRC_DIR}/skip_to_body.cpp")
set_property(TARGET server PROPERTY CXX_STANDARD 17)
target_include_directories(server PRIVATE "${WANGLE_INCLUDE_DIR}" "${COMPSKY_INCLUDE_DIRS}" "${PROJECT_INCLUDE_DIR}")
target_link_libraries(server wangle::wangle mariadbclient curl)
if(n_cached)
	target_compile_definitions(server PRIVATE "n_cached=${n_cached}")
endif()

add_custom_target(
	auto__server_determine_response_hpp ALL
	DEPENDS "${SRC_DIR}/server-endpoints.txt" # CMake does not seem to have an option to only rerun when dependency is modified
	COMMAND ${PYTHON_EXECUTABLE} "${SRC_DIR}/triegen.py" "${SRC_DIR}/server-endpoints.txt" "${SRC_DIR}/auto__server-determine-response.hpp"
	BYPRODUCTS auto__server-determine-response.hpp
	COMMENT "Generating bigboi switch statement"
)

add_custom_target(
	qry__process_arg__tokens ALL
	DEPENDS "${SRC_DIR}/server-endpoints.txt" # CMake does not seem to have an option to only rerun when dependency is modified
	COMMAND ${PYTHON_EXECUTABLE} "${SRC_DIR}/triegen.py" "${SRC_DIR}/qry-process_arg-tokens.txt" "${SRC_DIR}/qry-process_arg-tokens.hpp"
	BYPRODUCTS qry-process_arg-tokens.hpp
	COMMENT "Generating bigboi switch statement"
)

add_custom_target(
	qry__get_attribute_name__tokens ALL
	DEPENDS "${SRC_DIR}/server-endpoints.txt" # CMake does not seem to have an option to only rerun when dependency is modified
	COMMAND ${PYTHON_EXECUTABLE} "${SRC_DIR}/triegen.py" "${SRC_DIR}/qry-get_attribute_name-tokens.txt" "${SRC_DIR}/qry-get_attribute_name-tokens.hpp"
	BYPRODUCTS qry-get_attribute_name-tokens.hpp
	COMMENT "Generating bigboi switch statement"
)

add_custom_target(
	js_hpp ALL
	COMMAND ${PYTHON_EXECUTABLE} "${SRC_DIR}/minjs.py" "ALL" "${SRC_DIR}/js.hpp" "${SRC_DIR}/static/"
	BYPRODUCTS js.hpp
	COMMENT "Generating minimised JavaScript header"
)

add_dependencies(server auto__server_determine_response_hpp js_hpp)
add_dependencies(qry qry__process_arg__tokens qry__get_attribute_name__tokens)
