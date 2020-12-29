/*
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
*/
#pragma once

#include "nullable_string_view.hpp"
#include <compsky/macros/str2switch.hpp>
#include <compsky/utils/ptrdiff.hpp>


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
		desired_cookie.sz = compsky::utils::ptrdiff(cookies, desired_cookie.data);
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
