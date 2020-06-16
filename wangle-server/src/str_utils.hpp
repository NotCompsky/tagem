#pragma once

#include <compsky/macros/likely.hpp>


constexpr
const char* skip_to(const char* s,  const char c){
	while(true){
		if (unlikely(*s == 0))
			return nullptr;
		if (*s == c)
			return s;
	}
}

constexpr
const char* get_comma_separated_ints(const char** str,  const char separator){
	const char* const start = *str;
	while(true){
		if (not is_integer(**str))
			return nullptr;
		do {
			++(*str);
		} while (is_integer(**str));
		
		if (**str == ','){
			++(*str);
			continue;
		}
		
		return (**str == separator) ? start : nullptr;
	}
}
