#pragma once

#include <compsky/macros/str2switch.hpp>
#include <compsky/macros/likely.hpp>


#define IS_STR_EQL(str_to_be_tested,length,name) \
	[](const char*& str)->bool { /* NOTE: str is guaranteed to be more than 0 characters long, as we have already guaranteed that it starts with the file id */ \
		STR2SWITCH(length,name,return true;) \
		return false; \
	}(str_to_be_tested)


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


#define SKIP_TO_HTTP_CONTENT \
	[](const char* str)->const char* { \
		while(true){ \
			if (likely(not IS_STR_EQL(str,4,"\r\n\r\n"))) continue; \
			if (unlikely(*str == 0)) return nullptr; \
			return str + 1; \
		} \
	}


#define SKIP_TO_AFTER_SUBSTR__NONULLTERMINATE(length,name) \
	/* Returns a pointer to the character immediately AFTER the substr */ \
	[](const char* str)->const char* { \
		while(true){ \
			if (likely(not IS_STR_EQL(str,length,name))) continue; \
			return str + 1; \
		} \
	}
#define STRING_VIEW_FROM_UP_TO(length,name) \
	[](const char* str,  const char c)->const std::string_view { \
		str = SKIP_TO_AFTER_SUBSTR__NONULLTERMINATE(length,name)(str); \
		const char* const begin = str; \
		while(*str != c) \
			++str; \
		return std::string_view(begin,  (uintptr_t)str - (uintptr_t)begin); \
	}
