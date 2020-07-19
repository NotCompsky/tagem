#pragma once


constexpr
const char* skip_to_body(const char* s){
	while(*(++s) != 0){
		if (*s != '\n')
			continue;
		if (*(++s) == '\r'){
			if (*(++s) == '\n'){
				if (*(++s) == '\r')
					++s;
				return s;
			}
		}
	}
	return nullptr;
}
