#pragma once


constexpr
bool is_valid_hex_char(const char c){
	switch(c){
		case '0' ... '9':
		case 'a' ... 'f':
			return true;
		default:
			return false;
	}
}

constexpr
bool is_integer(const char c){
	return ((c >= '0')  and  (c <= '9'));
}
