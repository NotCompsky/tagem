#pragma once


namespace _r {
	constexpr static const std::string_view not_found =
		#include "headers/return_code/NOT_FOUND.c"
		"\n"
		"Not Found"
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
	
	constexpr static const std::string_view img_not_found(
		#include "headers/return_code/OK.c" // To encourage browsers to cache it.
		#include "headers/Content-Type/png.c"
		#include "headers/Cache-Control/1day.c"
		"Content-Length: 195\n"
		"\n"
		#include "i/404.txt" // WARNING: Corrupt, because the end of transmission character is skipped in this 'raw' string. // TODO: Figure out why
		, std::char_traits<char>::length(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/png.c"
			#include "headers/Cache-Control/1day.c"
			"Content-Length: 195\n"
			"\n"
		) + 195
	);
	
	constexpr static const std::string_view EMPTY_JSON_LIST = 
		#include "headers/return_code/OK.c" // To encourage browsers to cache it.
		#include "headers/Content-Type/json.c"
		#include "headers/Cache-Control/1day.c"
		"Content-Length: 2\n"
		"\n"
		"[]"
	;
	
	constexpr
	std::string_view return_static(const char* s){
		switch(*(s++)){
			case 'a':
				// a.css
				return
					#include "headers/return_code/OK.c"
					#include "headers/Content-Type/css.c"
					#include "headers/Cache-Control/1day.c"
					"\n"
					#include "static/all.css"
				;
			case 't':
				switch(*(s++)){
					case 'b':
						switch(*(s++)){
							case 'l':
								switch(*(s++)){
									case '1':
										// tbl1.css
										return
											#include "headers/return_code/OK.c"
											#include "headers/Content-Type/css.c"
											#include "headers/Cache-Control/1day.c"
											"\n"
											#include "static/table_as_blocks.css"
										;
									case '2':
										// tbl2.css
										return
											#include "headers/return_code/OK.c"
											#include "headers/Content-Type/css.c"
											#include "headers/Cache-Control/1day.c"
											"\n"
											#include "static/table_as_table.css"
										;
								}
								break;
						}
						break;
				}
				break;
			case 'u':
				switch(*(s++)){
					case '.':
						switch(*(s++)){
							case 'j':
								switch(*(s++)){
									case 's':
										switch(*(s++)){
											case ' ':
												return
													#include "headers/return_code/OK.c"
													#include "headers/Content-Type/javascript.c"
													#include "headers/Cache-Control/1day.c"
													"\n"
													#include "static/utils.js"
												;
										}
										break;
								}
								break;
						}
						break;
				}
				break;
		}
		return _r::not_found;
	}
} // namespace _r
