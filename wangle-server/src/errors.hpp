#pragma once


#include <cstdlib> // for exit


enum {
	NAH_NO_ERROR,
	MISC_ERROR,
	
	
	
	N_ERRORS
};

#ifdef NO_EXCEPTIONS
# define LOG(...)
#else
# include <cstdio>
# define LOG(...) fprintf(stderr, __VA_ARGS__); fflush(stderr);
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
	fprintf(stderr, "%s\n", handler_msgs[rc]);
	fflush(stderr);
   #endif
	exit(rc);
  #endif
}

inline
void log(const char* const str){
	LOG("log %s\n", str)
}
inline
void log(const size_t n){
	LOG("log %lu\n", n)
}
template<typename... Args,  typename T>
void handler(const int msg,  const T arg,  Args... args){
	log(arg);
	handler(msg, args...);
}
