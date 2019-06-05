#ifndef __COMPSKY_ASCIIFY__
#define __COMPSKY_ASCIIFY__

#include <string.h> // for memcpy
#include <inttypes.h> // for u?int[0-9]{1,2}_t

#include "utils.hpp" // for compsky::utils::*

namespace compsky::asciify {

extern char* BUF;
int BUF_INDX;

namespace flag {
    namespace concat {
        struct Start{};
        struct End{};
        const Start start;
        const End end;
    }
    struct ChangeBuffer{};
    const ChangeBuffer change_buffer;
    struct Escape{};
    const Escape escape;
    struct StrLen{};
    const StrLen strlen;
    struct FillWithLeadingZeros{};
    const FillWithLeadingZeros fill_with_leading_zeros;
    struct UpToFirstZero{};
    const UpToFirstZero uptofirstzero;
    namespace ensure {
        struct BetweenZeroAndOne{};
        const BetweenZeroAndOne between_zero_and_one;
    }
    namespace guarantee {
        struct BetweenZeroAndOne{};
        const BetweenZeroAndOne between_zero_and_one;
    }
}
namespace fake_type {
    struct Infinity{};
    const Infinity infinity;
}


/* Default cases */
template<typename... Args>
void asciify(uint64_t t,  Args... args);
template<typename... Args>
void asciify(int64_t t,  Args... args);
template<typename... Args>
void asciify(uint32_t t,  Args... args);
template<typename... Args>
void asciify(int32_t t,  Args... args);
template<typename... Args>
void asciify(uint16_t t,  Args... args);
template<typename... Args>
void asciify(int16_t t,  Args... args);
template<typename... Args>
void asciify(uint8_t t,  Args... args);
template<typename... Args>
void asciify(int8_t t,  Args... args);

template<typename... Args>
void asciify(const char c,  Args... args);

template<typename... Args>
void asciify(const char* s,  Args... args);

template<typename... Args>
void asciify(const char** s,  const int n,  Args... args);

/* Base Cases */
void asciify();
void asciify(const char* s);
void asciify(const char c);
void asciify(flag::StrLen f,  const char* s,  const int sz);



/* Headers */
template<typename... Args>
void asciify(flag::ChangeBuffer f,  char* buf,  int indx,  Args... args);

template<typename... Args>
void asciify(Args... args);

template<typename T>
void asciify_integer(T n);

template<typename T,  typename... Args>
void asciify(flag::FillWithLeadingZeros f,  const int min_digits,  T n,  Args... args);

template<typename T>
bool operator <(T t,  fake_type::Infinity x);

template<typename T>
bool operator >(fake_type::Infinity x,  T t);

template<typename Precision>
void asciify_subzero(double d,  Precision precision);

template<typename T,  typename P,  typename... Args>
void asciify_floaty(T d,  P precision);

template<typename T,  typename... Args>
void asciify(double d,  T precision,  Args... args);

template<typename T,  typename... Args>
void asciify(float f,  T precision,  Args... args);

template<typename T,  typename... Args>
void asciify(flag::guarantee::BetweenZeroAndOne f,  double d,  T precision,  Args... args);

template<typename T,  typename... Args>
void asciify(flag::Escape f,  const char c,  T s,  Args... args);

template<typename T,  typename... Args>
void asciify(flag::concat::Start f,  const char* s,  const int sz,  T t,  Args... args);

template<typename... Args>
void asciify(flag::concat::Start e,  const char* s,  const int sz,  flag::concat::End f,  Args... args);

template<typename T,  typename... Args>
void asciify(flag::concat::Start f,  const char c,  T t,  Args... args);

template<typename T,  typename S,  typename... Args>
void asciify(flag::concat::Start f,  const char c,  flag::guarantee::BetweenZeroAndOne g,  T t,  S s,  Args... args);

template<typename... Args>
void asciify(flag::concat::Start e,  const char c,  flag::concat::End f,  Args... args);




/* Base Case to Override (must precede Base Cases) */
template<typename... Args>
void asciify(uint64_t t,  Args... args){
    asciify_integer(t);
    return asciify(args...);
};
template<typename... Args>
void asciify(int64_t t,  Args... args){
    asciify_integer(t);
    return asciify(args...);
};
template<typename... Args>
void asciify(uint32_t t,  Args... args){
    asciify_integer(t);
    return asciify(args...);
};
template<typename... Args>
void asciify(int32_t t,  Args... args){
    asciify_integer(t);
    return asciify(args...);
};
template<typename... Args>
void asciify(uint16_t t,  Args... args){
    asciify_integer(t);
    return asciify(args...);
};
template<typename... Args>
void asciify(int16_t t,  Args... args){
    asciify_integer(t);
    return asciify(args...);
};
template<typename... Args>
void asciify(uint8_t t,  Args... args){
    asciify_integer(t);
    return asciify(args...);
};
template<typename... Args>
void asciify(int8_t t,  Args... args){
    asciify_integer(t);
    return asciify(args...);
};

template<typename... Args>
void asciify(const char c,  Args... args){
    asciify(c);
    return asciify(args...);
};

template<typename... Args>
void asciify(const char* s,  Args... args){
    asciify(s);
    return asciify(args...);
};

template<typename... Args>
void asciify(const char** s,  const int n,  Args... args){
    for (auto i = 0;  i < n;  ++i)
        asciify(s);
    return asciify(args...);
};

#ifdef QOBJECT
template<typename... Args>
void asciify(const QString& qs,  Args... args){
    QByteArray bs = qs.toLocal8Bit();
    const char* s = bs.data();
    return asciify(s, args...);
};
#endif

/*
template<typename T,  typename... Args>
void asciify(T t,  Args... args){
    if constexpr (
        // I am told: ‘if constexpr’ only available with -std=c++1z or -std=gnu++1z
        // But it *greatly* simplifies the code
        (std::is_same<T, uint64_t>::value) ||
        (std::is_same<T, int64_t>::value) ||
        (std::is_same<T, uint32_t>::value) ||
        (std::is_same<T, int32_t>::value) ||
        (std::is_same<T, uint16_t>::value) ||
        (std::is_same<T, int16_t>::value) ||
        (std::is_same<T, uint8_t>::value) ||
        (std::is_same<T, int8_t>::value)
    ){
        asciify_integer(t);
        return asciify(args...);
    }
    if constexpr (
        (std::is_same<T, const char*>::value)
    ){
        asciify(t);
        return asciify(args...);
    }
};
*/


/* Base Cases */
void asciify(){}

void asciify(const char* s){
    for (auto i = 0;  s[i] != 0;  ++i){
        BUF[BUF_INDX++] = s[i];
    }
}

void asciify(const char c){
    BUF[BUF_INDX++] = c;
}

void asciify(flag::StrLen f,  const char* s,  const int sz){
    memcpy(BUF,  s,  sz);
    BUF_INDX += sz;
};


/* Base Integer Cases */
template<typename T>
void asciify_integer(T n){
    auto n_digits = compsky::utils::count_digits(n);
    auto i = n_digits;
    BUF_INDX += i;
    do {
        BUF[--BUF_INDX] = '0' + (n % 10);
        n /= 10;
    } while (n != 0);
    BUF_INDX += n_digits;
};


/* Initialise Buffer */
template<typename... Args>
void asciify(flag::ChangeBuffer f,  char* buf,  int indx,  Args... args){
    BUF = buf;
    BUF_INDX = indx;
    asciify(args...);
};





template<typename T,  typename... Args>
void asciify(flag::FillWithLeadingZeros f,  const int min_digits,  T n,  Args... args){
    int n_digits = compsky::utils::count_digits(n);
    for (auto i = n_digits;  i < min_digits;  ++i)
        BUF[BUF_INDX++] = '0';
    asciify(n, args...);
};

template<typename T>
bool operator <(T t,  fake_type::Infinity x){
    return true;
};

template<typename T>
bool operator >(fake_type::Infinity x,  T t){
    return false;
};

template<typename Precision>
void asciify_subzero(double d,  Precision precision){
    for (auto i = 0;  d > 0  &&  i < precision;  ++i){
        d *= 10;
        char c = d;
        asciify((char)('0' + c));
        d -= c;
    }
};

template<typename T,  typename P,  typename... Args>
void asciify_floaty(T d,  P precision){
    if (d < 0)
        return asciify(-d, precision);
    asciify((uint64_t)d);
    d -= (uint64_t)d;
    asciify('.');
    asciify_subzero(d, precision);
};

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

template<typename T,  typename... Args>
void asciify(flag::guarantee::BetweenZeroAndOne f,  double d,  T precision,  Args... args){
    asciify((char)('0' + (char)d),  '.');
    d -= (char)d;
    asciify_subzero(d, precision);
    asciify(args...);
};

template<typename T,  typename... Args>
void asciify(flag::Escape f,  const char c,  T s,  Args... args){
    while(*s != 0){
        if (*s == c  ||  *s == '\\')
            BUF[BUF_INDX++] = '\\';
        BUF[BUF_INDX++] = *s;
        ++s;
    }
    asciify(args...);
};






template<typename T,  typename... Args>
void asciify(flag::concat::Start f,  const char* s,  const int sz,  T t,  Args... args){
    asciify(t);
    asciify(flag::strlen, s, sz);
    asciify(f, s, sz, args...);
};

template<typename... Args>
void asciify(flag::concat::Start e,  const char* s,  const int sz,  flag::concat::End f,  Args... args){
    // Overrides previous (more general) template
    BUF_INDX -= sz;
    asciify(args...);
};

template<typename... Args>
void asciify(flag::concat::Start e,  const char c,  flag::concat::End f,  Args... args){
    --BUF_INDX;
    asciify(args...);
};

template<typename T,  typename... Args>
void asciify(flag::concat::Start f,  const char c,  T t,  Args... args){
    asciify(t);
    asciify(c);
    asciify(f, c, args...);
};

template<typename T,  typename Precision,  typename... Args>
void asciify(flag::concat::Start f,  const char c,  flag::guarantee::BetweenZeroAndOne g,  T t,  Precision precision,  Args... args){
    asciify(g, t, precision);
    asciify(c);
    asciify(f, c, args...);
};


} // END: namespace compsky::asciify


#endif
