#pragma once

#include "read_request.hpp"
#include "nullable_string_view.hpp"
#include <compsky/macros/str2switch.hpp>


constexpr
NullableStringView get_cookie(const char* headers,  const char* const cookie_name){
	// NOTE: cookie_name should include the equals sign
	const char* cookies = SKIP_TO_HEADER(8,"Cookie: ")(headers);
	NullableStringView desired_cookie;
	if (cookies == nullptr){
		return desired_cookie;
	}
	while(*(++cookies) != 0){
		const char* cookie_name_itr = cookie_name;
		while(*cookies == *cookie_name_itr){
			++cookie_name_itr;
			++cookies;
			if(*cookie_name_itr == 0)
				desired_cookie.data = cookies;
		}
		
		// Skip to next cookie name
		while((*cookies != ';') and (*cookies != 0) and (*cookies != '\r'))
			++cookies;
		desired_cookie.sz = (uintptr_t)cookies - (uintptr_t)desired_cookie.data;
		switch(*cookies){
			case 0:
				// Probably invalid end of headers
			case '\r':
				// No more cookies
				return desired_cookie;
			default: // ';'
				switch(*(++cookies)){
					case ' ':
						// Good
						break;
					default:
						return desired_cookie;
				}
		}
		if (desired_cookie.data != nullptr)
			break;
	}
	return desired_cookie;
}
