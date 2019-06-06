#include <cstddef> // for size_t
#include <string.h> // for memset

#include "utils.hpp"


namespace compsky::utils {
static inline void
memzero_secure(void* data,  size_t len){
    // Based on work by Zhaomo Yang, of the University of California, who released it into the public domain.
  #if defined(_WIN32)
    SecureZeroMemory (data, len);
  #elif defined(__GNUC__) || defined(__clang__)
    memset(data, 0, len);
    __asm__ __volatile__("" : : "r"(data) : "memory");
  #else
    volatile char *p = (volatile char *) data;
    while (len--)
      *p++ = 0;
  #endif
}

template<typename T>
int count_digits(T m){
    int i = 0;
    T n = m;
    do {
        n /= 10;
        ++i;
    } while (n != 0);
    // Using do{}while() loop rather than while() loop avoids issue with special case of an input of 0 with the latter
    return i;
};


} // END: namespace compsky::utils
