#pragma once


#include <cstdlib> // for exit
#include "log.hpp"


enum {
	NAH_NO_ERROR,
	MISC_ERROR,
	
	
	
	N_ERRORS
};

#ifdef NO_EXCEPTIONS
# define LOG(...)
#else
# include <cstdio>
constexpr static
const char* const handler_msgs[] = {
	"No error",
	"Misc error",
	""
};
#endif

inline
void handler(const int rc){
	// Do nothing on a bad test result in order for the test itself to be optimised out
  #ifdef TESTS
   #ifndef NO_EXCEPTIONS
	compsky::os::write::write_to_stderr(handler_msgs[rc]);
   #endif
	exit(rc);
  #endif
}

template<typename... Args>
void handler(const int msg,  Args... args){
	log(args...);
	handler(msg);
}
