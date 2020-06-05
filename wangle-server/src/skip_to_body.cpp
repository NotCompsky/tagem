#include "skip_to_body.hpp"


bool skip_to_body(const char** s){
	// Find two consecutive newlines
	constexpr char data_link_escape = 16;
	while(true){
		while((**s != '\n') and (**s != 0))
			++(*s);
		if (**s == 0)
			return true;
		++(*s);
		
		if (**s == '\r'){
			++(*s);
			if (**s == '\n'){
				++(*s);
				return false;
			}
		}
	}
}
