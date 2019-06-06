#include <cstddef> // for size_t
#include <stdint.h> // for uintN_t

#include "asciify_flags.hpp" // for compsky::asciify::flag::*

// Assumes mysql/mysql.h, utils.hpp and mymysql.hpp are included in main scope of main program


void exec_buffer(const char* s,  const size_t sz);

void exec_buffer(const char* s);

template<typename... Args>
void exec(Args... args);




void query_buffer(const char* s,  const size_t sz);

void query_buffer(const char* s);

template<typename... Args>
void query(Args... args);


void assign_next_column();





template<typename T>
T ascii2n(T m);


namespace flag {
    struct SizeOfAssigned{};
}

template<typename... Args>
void assign_next_column(flag::SizeOfAssigned*& f,  size_t*& sz,  Args... args);

template<typename... Args>
void assign_next_column(uint64_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(int64_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(uint32_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(int32_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(uint16_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(int16_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(uint8_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(int8_t*& n,  Args... args);

template<typename... Args>
void assign_next_column(char**& s,  Args... args);

template<typename... Args>
void assign_next_column(compsky::asciify::flag::guarantee::BetweenZeroAndOne f,  double*& d,  Args... args);


void free_result();

template<typename... Args>
bool assign_next_result(Args... args);
