cmake_minimum_required(VERSION 3.9) # CMP0069 NEW

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Type")
option(EMBED_PYTHON "Allows the use of embedded Python, instead of relying on forking the process to call external Python programs" OFF)
option(MANGLE_JS_NAMES "Minimise the names of JavaScript functions and variables" OFF)
option(DISPLAY_LICENSES_INLINE "Display static component licenses inline" ON)
option(ENABLE_STATIC "Build static executable, rather than linked" OFF)
option(ENABLE_LIMIT_WITHIN_GROUP_CONCAT "Allow the use of LIMIT statements within GROUP_CONCAT functions, disabling the workaround. Currently only MariaDB servers have fixed this bug, while all official MySQL versions require the workaround." OFF)
option(BUILD_TESTS "Run compile-time tests" OFF)
option(USE_LOCAL_JS "Use local JavaScript, except for youtube" OFF)
option(USE_LOCAL_YT_JS "Use local Youtube JavaScript file at static/js/3rd-party/www.youtube.com/iframe_api, rather than fetching from youtube.com" OFF)
option(MONKEYPATCH_MY_SHA512 "Only for static builds. Linking against MariaDB requires this function to be defined, even though it is not used at all" ON)
option(ENABLE_SPREXER "Enables the info-extractor, which grabs information such as timestamp, author, title from web pages. Requires lexbor to be installed." ON)
option(ENABLE_CURL "Use libcurl instead of custom client based on boost::asio. This allows for downloading files from non-HTTP protocols, such as ftp" ON)
set(MAX_CACHE_AGE "0" CACHE STRING "Maximum age - in seconds - of the HTML and AJAX in the browser cache")
set(TABLE_LIMIT "100" CACHE STRING "Maximum number of entries in file, tag, dir tables")
set(DNS_OVER_HTTPS_CLIENT_URL "" CACHE STRING "URL of the web page to request DNS over HTTPS - e.g. https://cloudflare-dns.com/dns-query")
set(N_THREADS "4" CACHE STRING "Number of threads the server will spawn")
set(HANDLER_BUF_SZ "11534336" CACHE STRING "Size of the buffer given to each handler - the default is 11MiB")
set(N_SELECT3_OPTS "20" CACHE STRING "Number of options to display for tag/dir/etc queries from 'select3' inputs")

if(MANGLE_JS_NAMES)
	set(MANGLE_JS_NAMES_ARGS "--mangle")
else()
	set(MANGLE_JS_NAMES_ARGS "")
endif()

find_package(Threads REQUIRED)
find_package(Compsky REQUIRED)
find_package(Boost REQUIRED)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
set(AUTO_SRC_DIR "${SRC_DIR}/auto-generated")
set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")

set(MONKEYPATCH_SOURCES)
if(ENABLE_STATIC)
	set(CURL_NAMES curl.a libcurl.a)
	set(LIBCRYPTO_NAMES crypto.a libcrypto.a)
	set(BOOST_THREAD_NAMES boost_thread.a libboost_thread.a)
	set(PTHREAD)
	set(FFMPEGTHUMBNAILER_NAMES ffmpegthumbnailer.a libffmpegthumbnailer.a)
	set(MYSQL_CLIENT_NAMES mariadbclient.a libmariadbclient.a mariadb/mariadbclient.a mariadb/libmariadbclient.a mysqlclient.a libmysqlclient.a mysql/mysqlclient.a mysql/libmysqlclient.a)
	set(LIBMAGIC_NAMES magic.a libmagic.a)
	set(LIBSSL_NAMES ssl.a libssl.a openssl.a libopenssl.a)
	find_library(LIBAVDEVICE NAMES avdevice.a libavdevice.a)
	find_library(LIBAVFILTER NAMES avfilter.a libavfilter.a)
	find_library(LIBAVFORMAT NAMES avformat.a libavformat.a)
	find_library(LIBAVCODEC NAMES avcodec.a libavcodec.a)
	find_library(LIBAVUTIL NAMES avutil.a libavutil.a)
	find_library(LIBPOSTPROC NAMES postproc.a libpostproc.a) # For adding movie symbols onto thumbnails
	find_library(LIB264 NAMES x264.a libx264.a) # For thumbnailing MP4s
	set(FFMPEG_THUMBNAILER_DEPS ${LIBAVDEVICE} ${LIBAVFILTER} ${LIBAVFORMAT} ${LIBAVCODEC} ${LIBAVUTIL} ${LIBPOSTPROC} ${LIB264})
	if(MONKEYPATCH_MY_SHA512)
		set(MONKEYPATCH_SOURCES "${SRC_DIR}/fix-missing-symbol.monkeypatch.cpp")
	endif()
	set(LEXBOR_NAMES lexbor.a liblexbor.a lexbor_static.a liblexbor_static.a)
else()
	set(CURL_NAMES curl)
	set(LIBCRYPTO_NAMES crypto)
	set(BOOST_THREAD_NAMES boost_thread)
	set(LIBMAGIC_NAMES magic)
	find_library(PTHREAD NAMES pthread)
	set(FFMPEGTHUMBNAILER_NAMES ffmpegthumbnailer)
	set(MYSQL_CLIENT_NAMES mariadbclient mysqlclient)
	set(LIBSSL_NAMES ssl openssl)
	set(FFMPEG_THUMBNAILER_DEPS)
	set(LEXBOR_NAMES lexbor)
endif()
if(ENABLE_CURL)
	find_library(CURL NAMES ${CURL_NAMES})
else()
	set(CURL)
endif()
find_library(LIBSSL NAMES ${LIBSSL_NAMES})
find_library(LIBCRYPTO NAMES ${LIBCRYPTO_NAMES})
find_library(BOOST_THREAD NAMES ${BOOST_THREAD_NAMES})
find_library(FFMPEGTHUMBNAILER NAMES ${FFMPEGTHUMBNAILER_NAMES})
find_library(MYSQL_CLIENT NAMES ${MYSQL_CLIENT_NAMES})
find_library(LIBMAGIC NAMES ${LIBMAGIC_NAMES})
if(ENABLE_SPREXER)
	find_library(LEXBOR NAMES ${LEXBOR_NAMES}) # sprexer dependency
else()
	set(LEXBOR)
endif()


file(MAKE_DIRECTORY "${AUTO_SRC_DIR}")
# Creates directory during build system generation

set(SPREXER_SOURCES "${SRC_DIR}/sprexer/element.cpp" "${SRC_DIR}/sprexer/parser.cpp")

add_executable(
	server
	"${SRC_DIR}/server.cpp"
	"${SRC_DIR}/qry.cpp"
	"${SRC_DIR}/curl_utils.cpp"
	"${SRC_DIR}/db_info.cpp"
	"${SRC_DIR}/initialise_tagem_db.cpp"
	${MONKEYPATCH_SOURCES}
	${SPREXER_SOURCES}
)
if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	include(CheckIPOSupported)
	check_ipo_supported(RESULT is_ipo_supported)
endif()
set(DO_LTO FALSE)
if(is_ipo_supported)
	set(DO_LTO TRUE)
endif()
set_target_properties(
	server
	PROPERTIES
		CXX_STANDARD 17
		INTERPROCEDURAL_OPTIMIZATION "${DO_LTO}"
		LINK_FLAGS_RELEASE -s
)
target_include_directories(server PRIVATE "${COMPSKY_INCLUDE_DIRS}" "${PROJECT_INCLUDE_DIR}")
target_link_libraries(server PRIVATE "${MYSQL_CLIENT}" ${CURL} "${LIBCRYPTO}" "${BOOST_THREAD}" ${PTHREAD} ${LIBSSL} "${FFMPEGTHUMBNAILER}" ${FFMPEG_THUMBNAILER_DEPS} "${LIBMAGIC}" ${LEXBOR})

target_compile_definitions(server PRIVATE "MAX_CACHE_AGE=\"${MAX_CACHE_AGE}\"" "N_THREADS=${N_THREADS}" "HANDLER_BUF_SZ=${HANDLER_BUF_SZ}")
if(DNS_OVER_HTTPS_CLIENT_URL)
	target_compile_definitions(server PRIVATE "DNS_OVER_HTTPS_CLIENT_URL=\"${DNS_OVER_HTTPS_CLIENT_URL}\"")
endif()
if(ENABLE_CURL)
	target_compile_definitions(server PRIVATE USE_LIBCURL)
endif()
if(ENABLE_SPREXER)
	target_compile_definitions(server PRIVATE ENABLE_SPREXER)
endif()
if(EMBED_PYTHON)
	target_include_directories(server PRIVATE "/usr/include/python3.5" "/usr/include/python3.6" "/usr/include/python3.7" "/usr/include/python3.8" "/usr/include/python3.9" "/usr/include/python3.10")
	target_compile_definitions(server PRIVATE "PYTHON")
	if(ENABLE_STATIC)
		set(LIBPYTHON_NAMES "python3.10.a" "libpython3.10.a" "python3.9.a" "libpython3.9.a" "python3.8.a" "libpython3.8.a" "python3.7.a" "libpython3.7.a" "python3.6.a" "libpython3.6.a" "python3.5.a" "libpython3.5.a")
	else()
		set(LIBPYTHON_NAMES "python3.10" "python3.9" "python3.8" "python3.7" "python3.6" "python3.5") # WARNING: I have not tested any versions prior to python3.8
	endif()
	find_library(LIBPYTHON NAMES ${LIBPYTHON_NAMES})
	target_link_libraries(server PRIVATE "${LIBPYTHON}")
endif()
if(ENABLE_LIMIT_WITHIN_GROUP_CONCAT)
	target_compile_definitions(server PRIVATE LIMITS_WITHIN_GROUP_CONCATS)
endif()
if(BUILD_TESTS)
	target_compile_definitions(server PRIVATE TESTS)
endif()
if(USE_LOCAL_YT_JS)
	target_compile_definitions(server PRIVATE "USE_LOCAL_YT_JS")
endif()
if(USE_LOCAL_JS)
	target_compile_definitions(server PRIVATE "USE_LOCAL_JS")
endif()


add_custom_target(
	auto__server_determine_response_hpp ALL
	DEPENDS "${SRC_DIR}/server-endpoints.txt" # CMake does not seem to have an option to only rerun when dependency is modified
	COMMAND ${PYTHON_EXECUTABLE} "${SCRIPTS_DIR}/triegen.py" "${SRC_DIR}/server-endpoints.txt" "${AUTO_SRC_DIR}/auto__server-determine-response.hpp"
	BYPRODUCTS "${AUTO_SRC_DIR}/auto__server-determine-response.hpp"
	COMMENT "Generating bigboi switch statement"
)

add_custom_target(
	auto__infoextractor_verify_hpp ALL
	DEPENDS "${SRC_DIR}/info_extractor-verify.txt" # CMake does not seem to have an option to only rerun when dependency is modified
	COMMAND ${PYTHON_EXECUTABLE} "${SCRIPTS_DIR}/triegen.py" "${SRC_DIR}/info_extractor-verify.txt" "${AUTO_SRC_DIR}/auto__info_extractor-verify.hpp"
	BYPRODUCTS "${AUTO_SRC_DIR}/auto__server-determine-response.hpp"
	COMMENT "Generating bigboi switch statement"
)

add_custom_target(
	auto__infoextractor_domainid_hpp ALL
	DEPENDS "${SRC_DIR}/info_extractor-domainid.txt" # CMake does not seem to have an option to only rerun when dependency is modified
	COMMAND ${PYTHON_EXECUTABLE} "${SCRIPTS_DIR}/triegen.py" "${SRC_DIR}/info_extractor-domainid.txt" "${AUTO_SRC_DIR}/auto__info_extractor-domainid.hpp"
	BYPRODUCTS "${AUTO_SRC_DIR}/auto__server-determine-response.hpp"
	COMMENT "Generating bigboi switch statement"
)

add_custom_target(
	js_hpp ALL
	COMMAND ${PYTHON_EXECUTABLE} "${SCRIPTS_DIR}/minjs.py" "${MANGLE_JS_NAMES_ARGS}" "ALL" "${AUTO_SRC_DIR}/js.hpp" "${SRC_DIR}/static/js/" #"${SRC_DIR}/static/js/3rd-party/datetimepicker.js"
	BYPRODUCTS "${AUTO_SRC_DIR}/js.hpp"
	COMMENT "Generating minimised JavaScript header"
)

add_custom_target(
	svg_hpp ALL
	COMMAND ${PYTHON_EXECUTABLE} "${SCRIPTS_DIR}/svggen.py" "${DISPLAY_LICENSES_INLINE_ARGS}" "${AUTO_SRC_DIR}/svg.hpp" "${SRC_DIR}/html/svg"
	BYPRODUCTS "${AUTO_SRC_DIR}/svg.hpp"
	COMMENT "Generating SVG icons header"
)

add_custom_target(
	_SRC_DIR_static_js_3rd_party_php_date_formatter_js ALL
	COMMAND "${SCRIPTS_DIR}/wget-and-escape" "${SRC_DIR}/static/js/3rd-party/php-date-formatter.js" "https://raw.githubusercontent.com/kartik-v/php-date-formatter/master/js/php-date-formatter.min.js" 
	BYPRODUCTS "${SRC_DIR}/static/js/3rd-party/php-date-formatter.js"
	COMMENT "Downloading datetimepicker.css"
)

add_custom_target(
	_SRC_DIR_static_js_3rd_party_datetimepicker_js ALL
	COMMAND "${SCRIPTS_DIR}/wget-and-escape" "${SRC_DIR}/static/js/3rd-party/datetimepicker.js" "https://raw.githubusercontent.com/xdan/datetimepicker/master/jquery.datetimepicker.js" 
	BYPRODUCTS "${SRC_DIR}/static/js/3rd-party/datetimepicker.js"
	COMMENT "Downloading datetimepicker.css"
)

add_custom_target(
	_SRC_DIR_static_css_3rd_party_datetimepicker_css ALL
	COMMAND "${SCRIPTS_DIR}/wget-and-escape" "${SRC_DIR}/static/css/3rd-party/datetimepicker.css" "https://raw.githubusercontent.com/xdan/datetimepicker/master/jquery.datetimepicker.css" 
	BYPRODUCTS "${SRC_DIR}/static/css/3rd-party/datetimepicker.css"
	COMMENT "Downloading datetimepicker.css"
)


add_executable(qry "${SRC_DIR}/qry-cli.cpp" "${SRC_DIR}/qry.cpp")
set_property(TARGET qry PROPERTY CXX_STANDARD 17)
target_include_directories(qry PRIVATE "${COMPSKY_INCLUDE_DIRS}")
target_compile_definitions(qry PRIVATE DEBUG)
add_custom_target(
    qry__process_arg__tokens ALL
    DEPENDS "${SRC_DIR}/server-endpoints.txt" # CMake does not seem to have an option to only rerun when dependency is modified
    COMMAND ${PYTHON_EXECUTABLE} "${SCRIPTS_DIR}/triegen.py" "${SRC_DIR}/qry-process_arg-tokens.txt" "${AUTO_SRC_DIR}/qry-process_arg-tokens.hpp"
    BYPRODUCTS "${AUTO_SRC_DIR}/qry-process_arg-tokens.hpp"
    COMMENT "Generating bigboi switch statement"
)
add_custom_target(
    qry__get_attribute_name__tokens ALL
    DEPENDS "${SRC_DIR}/server-endpoints.txt" # CMake does not seem to have an option to only rerun when dependency is modified
    COMMAND ${PYTHON_EXECUTABLE} "${SCRIPTS_DIR}/triegen.py" "${SRC_DIR}/qry-get_attribute_name-tokens.txt" "${AUTO_SRC_DIR}/qry-get_attribute_name-tokens.hpp"
    BYPRODUCTS "${AUTO_SRC_DIR}/qry-get_attribute_name-tokens.hpp"
    COMMENT "Generating bigboi switch statement"
)


set(DISPLAY_LICENSES_INLINE_ARGS)
if(DISPLAY_LICENSES_INLINE)
	set(DISPLAY_LICENSES_INLINE_ARGS "--display-licences")
endif()

if(PREPOPULATE_T_TBL_FROM_IDS)
    target_compile_definitions(server PRIVATE PREPOPULATE_T_TBL_FROM_IDS="\\\"${PREPOPULATE_T_TBL_FROM_IDS}\\\"")
endif()
if(PREPOPULATE_D_TBL_FROM_IDS)
    target_compile_definitions(server PRIVATE PREPOPULATE_D_TBL_FROM_IDS="\\\"${PREPOPULATE_D_TBL_FROM_IDS}\\\"")
endif()
if(PREPOPULATE_F_TBL_FROM_IDS)
    # WARNING: An input value of "'0'" counts as false...
    target_compile_definitions(server PRIVATE PREPOPULATE_F_TBL_FROM_IDS="\\\"${PREPOPULATE_F_TBL_FROM_IDS}\\\"")
    # Why so many escapes? Not sure, but it works.
endif()
target_compile_definitions(server PRIVATE "N_SELECT3_OPTS=${N_SELECT3_OPTS}")
add_dependencies(qry qry__process_arg__tokens qry__get_attribute_name__tokens)


add_dependencies(server auto__server_determine_response_hpp auto__infoextractor_verify_hpp auto__infoextractor_domainid_hpp js_hpp svg_hpp qry__process_arg__tokens qry__get_attribute_name__tokens _SRC_DIR_static_js_3rd_party_php_date_formatter_js _SRC_DIR_static_js_3rd_party_datetimepicker_js _SRC_DIR_static_css_3rd_party_datetimepicker_css)

target_compile_definitions(server PRIVATE "TABLE_LIMIT=\"${TABLE_LIMIT}\"")
