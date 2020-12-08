#pragma once

#define COMPSKY_STD_STRING_VIEW
#include <compsky/asciify/asciify.hpp>
#include <compsky/os/write.hpp>


template<typename... Args>
void log(Args... args){
	char buf[1024];
	compsky::os::write_to_stderr(buf, args..., '\n');
}
