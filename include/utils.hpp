#ifndef __COMPSKY_UTILS__
#define __COMPSKY_UTILS__


namespace compsky::utils {
void memzero_secure(void* data,  size_t len);

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


#endif
