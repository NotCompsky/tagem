#pragma once


namespace _r {
	constexpr static const std::string_view not_found =
		#include "headers/return_code/NOT_FOUND.c"
		"\n"
		"Not Found"
	;
	
	constexpr static const std::string_view unauthorised =
		#include "headers/return_code/UNAUTHORISED.c"
		"\n"
		"Not Authorised"
	;
	
	constexpr static const std::string_view requires_login =
		#include "headers/return_code/UNAUTHORISED.c"
		"\n"
		"Requires login"
	;
	
	constexpr static const std::string_view invalid_file = 
		#include "headers/return_code/NOT_FOUND.c"
		"\n"
		"File does not exist"
	;
	
	constexpr static const std::string_view server_error =
		#include "headers/return_code/SERVER_ERROR.c"
		"\n"
		"Server error"
	;
	
	constexpr static const std::string_view not_implemented_yet =
		#include "headers/return_code/SERVER_ERROR.c"
		"\n"
		"Not implemented yet"
	;
	
	constexpr static const std::string_view post_ok =
		#include "headers/return_code/OK.c"
		#include "headers/Content-Type/text.c"
		"\n"
		"OK"
	;
	
	constexpr static const std::string_view post_not_necessarily_malicious_but_invalid =
		#include "headers/return_code/UNPROCESSABLE_ENTITY.c"
		#include "headers/Content-Type/text.c"
		"\n"
		"Invalid syntax"
	;
	
	constexpr static const std::string_view expect_100_continue =
		#include "headers/return_code/CONTINUE.c"
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
		"\n"
		"Your IP address has been temporarily banned"
	;
	
	constexpr static const char* const json_init =
		#include "headers/return_code/OK.c"
		#include "headers/Content-Type/json.c"
		"\n"
	;
	
	constexpr static const std::string_view EMPTY_JSON_LIST = 
		#include "headers/return_code/OK.c" // To encourage browsers to cache it.
		#include "headers/Content-Type/json.c"
		#include "headers/Cache-Control/1day.c"
		"Content-Length: 2\n"
		"\n"
		"[]"
	;
} // namespace _r
