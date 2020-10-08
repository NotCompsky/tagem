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

#include "verify_str.hpp"
#include "test.hpp"
#include "os.hpp"
#include <compsky/macros/likely.hpp>
#include <compsky/macros/str2switch.hpp>
#include <inttypes.h> // for uintptr_t


#define IS_STR_EQL(str_to_be_tested,length,name) \
	[](const char*& str)->bool { /* NOTE: str is guaranteed to be more than 0 characters long, as we have already guaranteed that it starts with the file id */ \
		STR2SWITCH(length,name,return true;) \
		return false; \
	}(str_to_be_tested)


constexpr
const char* skip_to(const char* s,  const char c){
	while(true){
		if (unlikely(*s == 0))
			return nullptr;
		if (*s == c)
			return s;
		++s;
	}
}

template<typename Char, size_t sz>
Char* skip_to_after(Char* s,  const char(&A)[sz]){
	do {
		const char* a = A;
		Char* const S = s;
		while(*s == *a){
			++s;
			++a;
			if (*a == 0)
				return S + sz - 1;
		}
	} while(*(++s) != 0);
	return nullptr;
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

constexpr
bool in_str(const char* str,  const char c){
	while(*str != 0){
		if (*str == c)
			return true;
		++str;
	}
	return false;
}
STATIC_ASSERT(in_str("foo",'f'));
STATIC_ASSERT(not in_str("bar",'f'));

constexpr
bool endswith(const char* str,  const char c){
	if (*str == 0)
		// Guarantee at least one iteration of the loop
		return false;
	while(*str != 0)
		++str;
	return (*(--str) == c);
}
STATIC_ASSERT(endswith("foo",'o'));
STATIC_ASSERT(not endswith("bar",'o'));

constexpr
void replace_first_instance_of(char* str,  const char a,  const char b){
	// str is guaranteed to be max characters long
	while(*str != 0){
		if(*str == a){
			*str = b;
			return;
		}
		++str;
	}
}

template<size_t N>
void replace_first_instance_of(char(&str)[N],  const char a,  const char b){
	// str is guaranteed to be max characters long
	for(size_t i = 0;  i < N;  ++i){
		if(str[i] == a){
			str[i] = b;
			return;
		}
	}
}

constexpr
void replace_first_instance_of(char* str,  const char a,  const char* b,  const char c){
	// str is guaranteed to be max characters long
	while(*str != 0){
		if(*str == a){
			while(*b != 0){
				*(str++) = *(b++);
			}
			// WARNING: No bounds checking (laziness)
			*str = c;
			return;
		}
	}
}

constexpr
size_t count_until(const char* s,  const char c){
	// NOTE: Not inclusive
	size_t n = 0;
	while((*s != c) and (*s != 0)){
		++s;
		++n;
	}
	return n;
}
STATIC_ASSERT(count_until("foobar",'b')==3);

constexpr
bool matches__left_up_to_space__right_up_to_comma_or_null(const char* const A,  const char* b){
	// WARNING: b should be guaranteed to either be empty, or contain non-empty strings delineated by commas
	while(*b != 0){
		const char* a = A;
		while((*b == *a) and (*a != ' ') and (*a != 0) and (*b != ',') and (*b != 0)){
			++b;
			++a;
		}
		if ((*a == ' ') and ((*b == ',') or (*b == 0)))
			return true;
		while((*b != ',') and (*b != 0))
			++b;
		if (*b == 0)
			return false;
		++b; // Skip the comma
	}
	return false;
}



constexpr
const char* basename__accepting_trailing_slash(const char* path){
	const char* fname = path;
	while(*path != 0){
		if (*path == os::unix_path_sep){
			if (*(++path) == 0)
				// NOTE: The next character won't be a slash
				break;
			// Do not set as start of fname if it is the end of the path
			fname = path;
		}
		++path;
	}
	return fname;
}



constexpr
bool char_in(const char c){
	return false;
}

template<typename... Args>
constexpr
bool char_in(const char c,  const char d,  Args... args){
	return (c == d) or char_in(c, args...);
}

template<typename... Args>
constexpr
size_t n_chars_until_char_in(const char* const str_orig,  Args... args){
	const char* str = str_orig;
	while(not char_in(*str, args...))
		++str;
	return (uintptr_t)str - (uintptr_t)str_orig;
}

STATIC_ASSERT(n_chars_until_char_in("foo/bar/ree/gee/", '/') == 3);
STATIC_ASSERT(n_chars_until_char_in("/bar/ree/gee/", '/') == 0);
STATIC_ASSERT(n_chars_until_char_in("/bar/ree/gee/", 'a', 'b', 'c') == 1);



template<typename... Args>
constexpr
bool in_str_not_at_end__where_end_marked_by(const char* str,  const char c,  Args... args){
	while(not char_in(*str, args...)){
		if (*str == c)
			return (not char_in(str[1], args...));
		++str;
	}
	return false;
}
constexpr
bool in_str_not_at_end(const char* const str,  const char c){
	return in_str_not_at_end__where_end_marked_by(str, c, '\0');
}
STATIC_ASSERT(in_str_not_at_end("www.youtube.com/watch?v=ABCDEFGHIJK", '/'));
STATIC_ASSERT(not in_str_not_at_end("watch?v=ABCDEFGHIJK", '/'));
STATIC_ASSERT(not in_str_not_at_end("foobar/", '/'));



constexpr
void get_file_name_and_ext__filename_ends_with_newline_or_null(const char* itr,  const char*& file_name,  const char*& ext){
	// WARNING: Assumes there is at least one slash in itr
	while((*itr != 0) and (*itr != '\n')){
		if ((itr[0] == os::unix_path_sep) and (itr[1] != 0) and (itr[1] != '\n'))
			file_name = itr;
		else if (*itr == '.')
			ext = itr;
		++itr;
	}
	++file_name; // Skip the slash
}
