#pragma once

#include <compsky/macros/str2switch.hpp>


#define TEST_IS_HEADER(length,name) \
	likely(not [](const char*& str)->bool { /* NOTE: str is guaranteed to be more than 0 characters long, as we have already guaranteed that it starts with the file id */ \
		STR2SWITCH(length,name,return true;) \
		return false; \
	}(str))

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
const char* skip_to_post_data(const char* s){
	while(*(++s) != 0){
		if (*s != '\n')
			continue;
		if (*(++s) == '\r')
			if (*(++s) == '\n')
				return s;
	}
	return nullptr;
}

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
