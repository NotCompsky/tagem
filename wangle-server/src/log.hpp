#pragma once

#define COMPSKY_STD_STRING_VIEW
#include <compsky/asciify/asciify.hpp>


template<typename... Args>
void log(Args... args){
	static char buf[1024];
	compsky::asciify::asciify(buf, args..., '\0');
	fprintf(stderr, "%s\n", buf);
	fflush(stderr);
}
