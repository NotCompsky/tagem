#ifndef __COMPSKY_UTILS__
#define __COMPSKY_UTILS__

#include <sys/types.h> // Linux/GNU
#include <string.h> // for memcpy
#include <inttypes.h> // for u?int[0-9]{1,2}_t

extern char* BUF;
extern int BUF_INDX;








/* Structs */

struct StartPrefixFlag{
    StartPrefixFlag(const char* str,  const int len) : str(str), len(len) {};
    const char* str;
    const int len;
};

struct EndPrefixFlag{};

struct StartConcatWith{
    StartConcatWith(const char* str,  const int len) : str(str), len(len) {};
    const char* str;
    const int len;
};
struct EndConcatWith{};




namespace flag {
	namespace concat {
        struct Start{};
        struct End{};
        Start start;
        End end;
	}
	StrLen{};
    StrLen strlen;
	FillWithLeadingZeros{};
    FillWithLeadingZeros fill_with_leading_zeros;
	UpToFirstZero{};
	UpToFirstZero uptofirstzero;
	namespace ensure {
		struct BetweenZeroAndOne;
		BetweenZeroAndOne between_zero_and_one;
	}
	namespace guarantee {
		struct BetweenZeroAndOne;
		BetweenZeroAndOne between_zero_and_one;
	}
}
namespace fake_type {
	struct Infinity{};
	Infinity infinity;
}



/*
Code and description based on work by Zhaomo Yang, of the University of California, who released it into the public domain.


*/
static inline void
memzero_secure(void* data,  size_t len){
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

/*
typedef void* (*memset_t)(void*, int, size_t);
static volatile memset_t memset_fnct = &memset;
void memzero_secure(void* ptr, size_t len){
    // Same as OPENSSL_cleanse - security described in https://www.usenix.org/system/files/conference/usenixsecurity17/sec17-yang.pdf (page 6)
    // Basically just a trick to confuse the compiler into not optimising the memset call away
    memset_fnct(ptr, 0, len);
}
*/







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



/* Base */
void asciify(const char* s){
    while (*s != 0){
        BUF[BUF_INDX++] = *s;
        ++s;
    }
}
void asciify(const char c){
    BUF[BUF_INDX++] = c;
}


template<typename T>
void asciify_integer(T n){
    auto n_digits = count_digits(n);
    auto i = n_digits;
    BUF_INDX += i;
    do {
        BUF[--BUF_INDX] = '0' + (n % 10);
        n /= 10;
    } while (n != 0);
    BUF_INDX += n_digits;
};
void asciify(uint64_t n){
    asciify_integer(n);
}
void asciify(int64_t n){
    asciify_integer(n);
}
void asciify(uint32_t n){
    asciify_integer(n);
}
void asciify(int32_t n){
    asciify_integer(n);
}
void asciify(uint16_t n){
    asciify_integer(n);
}
void asciify(int16_t n){
    asciify_integer(n);
}
void asciify(uint8_t n){
    asciify_integer(n);
}
void asciify(int8_t n){
    asciify_integer(n);
}





/* Headers */
template<typename... Args>
void asciify(Args... args);






template<typename T,  typename... Args>
void asciify(flag::FillWithLeadingZeros f,  const int min_digits,  T n,  Args... args){
    int n_digits = count_digits(n);
    for (auto i = n_digits;  i < min_digits;  ++i)
        BUF[BUF_INDX++] = '0';
    asciify(n, args...);
};






template<typename T>
void asciify_floaty(T dd,  fake_type::Infinity f){
    if (dd < 0){
        BUF[BUF_INDX++] = '-';
        return asciify_floaty(-dd);
    }
    T d = dd;
    int magnitude = 0;
    uint64_t scale = 1;
    do {
        ++magnitude;
        scale *= 10;
        d /= 10;
    } while (d >= 1);
    
    for (auto i = 0;  i < magnitude;  ++i){
        d *= 10;
        const char m = d;
        BUF[BUF_INDX++] = '0' + m;
        d -= m;
    }
    
    BUF[BUF_INDX++] = '.';
    
    for (auto i = 0;  d > 0;  ++i){
        d *= 10;
        char m = (char)d;
        BUF[BUF_INDX++] = '0' + m;
        d -= m;
    }
};

template<typename T>
void asciify_floaty(T dd,  int precision){
    if (dd < 0){
        BUF[BUF_INDX++] = '-';
        return asciify_floaty(-dd, precision);
    }
    T d = dd;
    int magnitude = 0;
    uint64_t scale = 1;
    do {
        ++magnitude;
        scale *= 10;
        d /= 10;
    } while (d >= 1);
    
    for (auto i = 0;  i < magnitude;  ++i){
        d *= 10;
        const char m = d;
        BUF[BUF_INDX++] = '0' + m;
        d -= m;
    }
    
    BUF[BUF_INDX++] = '.';
    
    for (auto i = 0;  i < precision;  ++i){
        d *= 10;
        char m = (char)d;
        BUF[BUF_INDX++] = '0' + m;
        d -= m;
    }
};

template<typename T>
void asciify_floaty(flag::UpToFirstZero f,  T dd){
    if (dd < 0){
        BUF[BUF_INDX++] = '-';
        return asciify_floaty(f, -dd);
    }
    T d = dd;
    int magnitude = 0;
    uint64_t scale = 1;
    do {
        ++magnitude;
        scale *= 10;
        d /= 10;
    } while (d >= 1);
    
    for (auto i = 0;  i < magnitude;  ++i){
        d *= 10;
        const char m = d;
        BUF[BUF_INDX++] = '0' + m;
        d -= m;
    }
    
    BUF[BUF_INDX++] = '.';
    
    char m;
    for (auto i = 0;  ;  ++i){
        d *= 10;
        char m = (char)d;
        BUF[BUF_INDX++] = '0' + m;
        if (m == 0)
            break;
        d -= m;
    } while (d > 0);
};

void asciify(){}

template<typename T,  typename... Args>
void asciify(double d,  T precision,  Args... args){
    asciify_floaty(d, precision);
    asciify(args...);
};

template<typename T,  typename... Args>
void asciify(float f,  T precision,  Args... args){
    asciify_floaty(f, precision);
    asciify(args...);
};












void asciify_subzero(double d,  int precision){
    for (auto i = 0;  i < precision;  ++i){
        d *= 10;
        char c = d;
        asciify('0' + c);
        d -= c;
    }
}

template<typename T>
void asciify(flag::guarantee::BetweenZeroAndOne f,  double d,  T precision){
    asciify('0' + (char)d,  '.');
    asciify_subzero(d, precision);
};

template<typename T,  typename... Args>
void asciify(flag::guarantee::BetweenZeroAndOne f,  double d,  T precision,  Args... args){
    asciify(f, d, precision);
    asciify(args...);
};




template<typename T,  typename... Args>
void asciify(double d,  T precision){
    if (d < 0)
        return asciify(-d, precision);
    asciify((uint64_t)d);
    d -= (uint64_t)d;
    asciify('.');
    asciify_subzero(d, precision);
};












void asciify(flag::concat::Start f,  const char* s,  const int sz,  const uint64_t n,  Args... args){
    asciify(n);
    asciify(flag::strlen, s, sz);
    asciify(f, s, sz, args...);
}

template<typename... Args>
void asciify(flag::concat::Start f,  const char* s,  const int sz,  const char* a,  Args... args){
    asciify(a);
    asciify(flag::strlen, s, sz);
    asciify(f, s, sz, args...);
};

template<typename... Args>
void asciify(flag::concat::Start e,  const char* s,  const int sz,  flag::concat::End f,  Args... args){
    // Overrides previous (more general) template
    --BUF_INDX;
    asciify(args...);
};


#endif
