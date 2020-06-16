#pragma once


template<typename Int>
Int a2n(const char* s){
	Int n = 0;
	while(*s >= '0'  &&  *s <= '9'){
		n *= 10;
		n += *s - '0';
		++s;
	}
	return n;
}

template<typename Int>
Int a2n(const char** s){
	Int n = 0;
	while(**s >= '0'  &&  **s <= '9'){
		n *= 10;
		n += **s - '0';
		++(*s);
	}
	return n;
}
