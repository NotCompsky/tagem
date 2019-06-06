#ifndef __COMPSKY__ASCIIFY__
#define __COMPSKY__ASCIIFY__

#include <inttypes.h> // for u?int[0-9]{1,2}_t

#include "asciify_flags.hpp"


namespace compsky::asciify {

extern char* BUF;
extern int BUF_INDX;

namespace fake_type {
    struct Infinity{};
}




/* Base Case to Override (must precede Base Cases) */
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

#ifdef QT_GUI_LIB
void asciify(const QString& qs);

template<typename... Args>
void asciify(const QString& qs,  Args... args);
#endif


/* Base Cases */
void asciify();

void asciify(const char* s);

void asciify(const char c);

void asciify(flag::StrLen f,  const char* s,  const int sz);


/* Base Integer Cases */
template<typename T>
void asciify_integer(T n);


/* Initialise Buffer */
template<typename... Args>
void asciify(flag::ChangeBuffer f,  char* buf,  int indx,  Args... args);





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
void asciify(flag::concat::Start f,  const char* s,  const int sz,  const char** ss,  T n,  Args... args);

template<typename T,  typename... Args>
void asciify(flag::concat::Start f,  const char* s,  const int sz,  const std::vector<const char*> ss,  T n,  Args... args);

template<typename... Args>
void asciify(flag::concat::Start e,  const char c,  flag::concat::End f,  Args... args);

template<typename T,  typename... Args>
void asciify(flag::concat::Start f,  const char c,  T t,  Args... args);

template<typename T,  typename Precision,  typename... Args>
void asciify(flag::concat::Start f,  const char c,  flag::guarantee::BetweenZeroAndOne g,  T t,  Precision precision,  Args... args);

} // END: namespace compsky::asciify

#endif
