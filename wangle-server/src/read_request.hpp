#pragma once

#include "str_utils.hpp"


#define TEST_IS_HEADER(length,name) \
	likely(not IS_STR_EQL(str,length,name))

#define SKIP_TO_HEADER(length,name) \
	/* Returns a pointer to the character immediately BEFORE the cookies list (i.e. the space character in "Cookie: "). This is a silly little micro-optimistaion. */ \
	[](const char* str)->const char* { \
		while(*(++str) != 0){ \
			if (*str != '\n') continue; \
			if (TEST_IS_HEADER(length,name)) continue; \
			if (unlikely(*str == 0)) continue; \
			return str; \
		} \
		return nullptr; \
	}


enum GetRangeHeaderResult {
	none,
	valid,
	invalid
};


constexpr
GetRangeHeaderResult get_range(const char* str,  size_t& from,  size_t& to){
	while(*(++str) != 0){ // NOTE: str is guaranteed to be more than 0 characters long, as we have already guaranteed that it starts with the file id
		if (*str != '\n')
			continue;
		if (TEST_IS_HEADER(13,"Range: bytes="))
			continue;
		if (*str == 0)
			break;
		
		++str; // Skip '=' - a2n is safe even if the next character is null
		from = a2n<size_t>(&str);
		
		if (*str != '-')
			return GetRangeHeaderResult::invalid;
		
		++str; // Skip '-' - a2n is safe even if the next character is null
		to = a2n<size_t>(str);
		
		return GetRangeHeaderResult::valid;
	}
	from = 0;
	to = 0;
	return GetRangeHeaderResult::none;
}
