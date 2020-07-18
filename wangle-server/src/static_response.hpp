#pragma once


#define HEADER__CONTENT_TYPE__JSON "Content-Type: application/json\n"
#define HEADER__CONTENT_TYPE__PNG  "Content-Type: image/png\n"
#define HEADER__CONTENT_TYPE__TEXT "Content-Type: text/plain\n"
#define HEADER__CONTENT_TYPE__ICO  "Content-Type: image/ico\n"
#define HEADER__CONTENT_TYPE__HTML "Content-Type: text/html\n"
#define HEADER__CONTENT_TYPE__CSS  "Content-Type: text/css\n"


namespace _r {
	constexpr static const std::string_view not_found =
		#include "headers/return_code/NOT_FOUND.c"
		HEADER__CONTENT_TYPE__TEXT
		"\n"
		"Not Found"
	;
	
	constexpr static const std::string_view unauthorised =
		#include "headers/return_code/UNAUTHORISED.c"
		HEADER__CONTENT_TYPE__TEXT
		"\n"
		"Not Authorised"
	;
	
	constexpr static const std::string_view requires_login =
		#include "headers/return_code/UNAUTHORISED.c"
		HEADER__CONTENT_TYPE__TEXT
		"\n"
		"Requires login"
	;
	
	constexpr static const std::string_view invalid_file = 
		#include "headers/return_code/NOT_FOUND.c"
		HEADER__CONTENT_TYPE__TEXT
		"\n"
		"File does not exist"
	;
	
	constexpr static const std::string_view server_error =
		#include "headers/return_code/SERVER_ERROR.c"
		HEADER__CONTENT_TYPE__TEXT
		"\n"
		"Server error"
	;
	
	constexpr static const std::string_view not_implemented_yet =
		#include "headers/return_code/SERVER_ERROR.c"
		HEADER__CONTENT_TYPE__TEXT
		"\n"
		"Not implemented yet"
	;
	
	constexpr static const std::string_view post_ok =
		#include "headers/return_code/OK.c"
		HEADER__CONTENT_TYPE__TEXT
		"\n"
		"OK"
	;
	
	constexpr static const std::string_view post_not_necessarily_malicious_but_invalid =
		#include "headers/return_code/UNPROCESSABLE_ENTITY.c"
		HEADER__CONTENT_TYPE__TEXT
		"\n"
		"Invalid syntax"
	;
	
	constexpr static const std::string_view expect_100_continue =
		#include "headers/return_code/CONTINUE.c"
		HEADER__CONTENT_TYPE__TEXT
		"\n"
		"why even bother asking"
	;
	
	/*
	constexpr static const std::string_view bad_request =
		#include "headers/return_code/BAD_REQUEST.c"
		"\n"
		"Bad Request"
	;
	*/
	constexpr static const std::string_view bad_request = not_found;
	
	constexpr static const std::string_view banned_client =
		#include "headers/return_code/UNAUTHORISED.c"
		// Dirty scoundrels don't even deserve a Content-Type header!!!
		"\n"
		"Your IP address has been temporarily banned"
	;
	
	constexpr static const char* const json_init =
		#include "headers/return_code/OK.c"
		HEADER__CONTENT_TYPE__JSON
		"\n"
	;
	
	constexpr static const std::string_view EMPTY_JSON_LIST = 
		#include "headers/return_code/OK.c" // To encourage browsers to cache it.
		HEADER__CONTENT_TYPE__JSON
		CACHE_CONTROL_HEADER
		"Content-Length: 2\n"
		"\n"
		"[]"
	;
} // namespace _r
