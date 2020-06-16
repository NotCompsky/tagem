#pragma once

#include "nullable_string_view.hpp"


constexpr
const char* get_cookies(const char* headers){
	// Returns a pointer to the character immediately BEFORE the cookies list (i.e. the space character in "Cookie: "). This is a silly little micro-optimistaion.
	while(*(++headers) != 0){ // NOTE: headers is guaranteed to be more than 0 characters long, as we have already guaranteed that it starts with the file id
		if (*headers != '\n')
			continue;
		bool is_cookie_header = false;
		switch(*(++headers)){
			case 'C':
				switch(*(++headers)){
					case 'o':
						switch(*(++headers)){
							case 'o':
								switch(*(++headers)){
									case 'k':
										switch(*(++headers)){
											case 'i':
												switch(*(++headers)){
													case 'e':
														switch(*(++headers)){
															case ':':
																switch(*(++headers)){
																	case ' ':
																		is_cookie_header = true;
																		break;
																}
																break;
														}
														break;
												}
												break;
										}
										break;
								}
								break;
						}
						break;
				}
				break;
		}
		if (unlikely(*headers == 0))
			break;
		if (likely(not is_cookie_header))
			continue;
		
		return headers;
	}
	return nullptr;
}

constexpr
NullableStringView get_cookie(const char* s,  const char* const cookie_name){
	// NOTE: cookie_name should include the equals sign
	const char* cookies = get_cookies(s);
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
		while((*cookies != ';') and (*cookies != 0))
			++cookies;
		desired_cookie.sz = (uintptr_t)cookies - (uintptr_t)desired_cookie.data;
		switch(*cookies){
			case 0:
				// Probably invalid end of headers
			case '\n':
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
