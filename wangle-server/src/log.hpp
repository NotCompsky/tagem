#pragma once

#define COMPSKY_STD_STRING_VIEW
#include <compsky/asciify/asciify.hpp>
#include <compsky/os/write.hpp>


template<typename... Args>
void log(Args... args){
	static char buf[1024];
	compsky::os::write_to_stderr(buf, args..., '\n');
}

template<typename... Args>
void static_log(Args... args){
	static char buf[1024 * 50];
	compsky::os::write_to_stderr(buf, args..., '\n');
}
