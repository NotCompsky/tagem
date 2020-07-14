#pragma once

#include <compsky/macros/likely.hpp>
#include <compsky/macros/str2switch.hpp>


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
static_assert(in_str("foo",'f'));
static_assert(not in_str("bar",'f'));

constexpr
bool endswith(const char* str,  const char c){
	if (*str == 0)
		// Guarantee at least one iteration of the loop
		return false;
	while(*str != 0)
		++str;
	return (*(--str) == c);
}
static_assert(endswith("foo",'o'));
static_assert(not endswith("bar",'o'));

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

template<size_t N>
void replace_first_instance_of(char(&str)[N],  const char a,  const char* b,  const char c){
	// str is guaranteed to be max characters long
	for(size_t i = 0;  i < N;  ++i){
		if(str[i] == a){
			while(*b != 0){
				str[i++] = *(b++);
			}
			// WARNING: No bounds checking (laziness)
			str[i] = c;
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
static_assert(count_until("foobar",'b')==3);

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
		if (*path == '/'){
			if (*(++path) == 0)
				break;
			// Do not set as start of fname if it is the end of the path
			fname = path;
		}
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

static_assert(n_chars_until_char_in("foo/bar/ree/gee/", '/') == 3);
static_assert(n_chars_until_char_in("/bar/ree/gee/", '/') == 0);
static_assert(n_chars_until_char_in("/bar/ree/gee/", 'a', 'b', 'c') == 1);



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
static_assert(in_str_not_at_end("www.youtube.com/watch?v=ABCDEFGHIJK", '/'));
static_assert(not in_str_not_at_end("watch?v=ABCDEFGHIJK", '/'));
static_assert(not in_str_not_at_end("foobar/", '/'));
