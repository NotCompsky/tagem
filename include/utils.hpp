#ifndef __COMPSKY_UTILS__
#define __COMPSKY_UTILS__


namespace compsky::utils {
static inline void
memzero_secure(void* data,  size_t len);

template<typename T>
int count_digits(T m);


} // END: namespace compsky::utils


#endif
