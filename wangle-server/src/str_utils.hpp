#pragma once

#include <compsky/macros/likely.hpp>


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
