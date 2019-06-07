#ifndef __MYMYSQL_RESULTS__
#define __MYMYSQL_RESULTS__

#include <cstddef> // for size_t
#include <stdint.h> // for uintN_t

#include "asciify_core.hpp" // for compsky::asciify::BUF_INDX
#include "asciify_flags.hpp" // for compsky::asciify::flag::*

#include "mymysql.hpp"

#ifdef DEBUG
  #include <stdio.h> // for printf
#endif


// Assumes mysql/mysql.h, utils.hpp and mymysql.hpp are included in main scope of main program

namespace mymysql {

template<typename... Args>
void exec(Args... args);

template<typename... Args>
void query(MYSQL_RES** res,  Args... args);





template<typename T>
T ascii2n(MYSQL_ROW row,  int* col,  T m);


template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  flag::SizeOfAssigned*& f,  size_t*& sz,  Args... args);

template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  uint64_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  int64_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  uint32_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  int32_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  uint16_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  int16_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  uint8_t*& n,  Args... args);
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  int8_t*& n,  Args... args);

template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  char**& s,  Args... args);

template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  compsky::asciify::flag::guarantee::BetweenZeroAndOne f,  double*& d,  Args... args);

template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  compsky::asciify::flag::guarantee::BetweenZeroAndOneInclusive f,  double*& d,  Args... args);

template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  compsky::asciify::flag::guarantee::BetweenZeroAndTenLeftInclusive f,  double*& d,  Args... args);

template<typename... Args>
bool assign_next_result(MYSQL_RES* res,  MYSQL_ROW* row,  Args... args);

} // END namespace mymysql














































#include <string.h> // for strlen

#include "asciify.hpp" // for compsky::asciify::*

#include "mymysql.hpp"


namespace mymysql {

template<typename... Args>
void exec(Args... args){
    compsky::asciify::asciify(compsky::asciify::flag::change_buffer, compsky::asciify::BUF, 0, args...);
  #ifdef DEBUG
    printf("%s\n", compsky::asciify::BUF);
  #endif
    exec_buffer(compsky::asciify::BUF, compsky::asciify::BUF_INDX);
};




template<typename... Args>
void query(MYSQL_RES** res,  Args... args){
    compsky::asciify::asciify(compsky::asciify::flag::change_buffer, compsky::asciify::BUF, 0, args...);
    query_buffer(res, compsky::asciify::BUF, compsky::asciify::BUF_INDX);
};





template<typename T>
T ascii2n(MYSQL_ROW row,  int col,  T m){
    T n = 0;
    char* s = row[col];
    while (*s != 0){
        n *= 10;
        n += *s - '0';
        ++s;
    }
    return n;
};



template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  flag::SizeOfAssigned*& f,  size_t*& sz,  Args... args){
    *sz = row[*col+1] - row[*col];
    return assign_next_column(row,  col,  args...);
};

template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  uint64_t*& n,  Args... args){
    *n = ascii2n(row, (*col)++, *n);
    assign_next_column(row,  col,  args...);
};
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  int64_t*& n,  Args... args){
    *n = ascii2n(row, (*col)++, *n);
    assign_next_column(row,  col,  args...);
};
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  uint32_t*& n,  Args... args){
    *n = ascii2n(row, (*col)++, *n);
    assign_next_column(row,  col,  args...);
};
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  int32_t*& n,  Args... args){
    *n = ascii2n(row, (*col)++, *n);
    assign_next_column(row,  col,  args...);
};
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  uint16_t*& n,  Args... args){
    *n = ascii2n(row, (*col)++, *n);
    assign_next_column(row,  col,  args...);
};
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  int16_t*& n,  Args... args){
    *n = ascii2n(row, (*col)++, *n);
    assign_next_column(row,  col,  args...);
};
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  uint8_t*& n,  Args... args){
    *n = ascii2n(row, (*col)++, *n);
    assign_next_column(row,  col,  args...);
};
template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  int8_t*& n,  Args... args){
    *n = ascii2n(row, (*col)++, *n);
    assign_next_column(row,  col,  args...);
};

template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  char**& s,  Args... args){
    *s = row[(*col)++];
    assign_next_column(row,  col,  args...);
};

template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  compsky::asciify::flag::guarantee::BetweenZeroAndOne f,  double*& d,  Args... args){
    char* s = row[(*col)++];
    ++s; // Skip the "0"
    
    *d = 0;
    
    if (*s == 0)
        return;
    
    ++s; // Skip the "."
    int n_digits = strlen(s);
    
    for (auto i = n_digits - 1;  i >= 0;  --i){
        *d += s[i] - '0';
        *d /= 10;
    }
    
    assign_next_column(row,  col,  args...);
};

template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  compsky::asciify::flag::guarantee::BetweenZeroAndOneInclusive f,  double*& d,  Args... args){
    // Mostly seperated from `BetweenZeroAndTenLeftInclusive` case for clarity - might be confusing to declare some things as between 0 and 10 when they are between 0 and 1.
    // Maybe some performance gain with `*d = (*s == '0') ? 0 : 1;`... but not worth it
    assign_next_column(row, col, compsky::asciify::flag::guarantee::between_zero_and_ten_left_inclusive, d, args...);
};

template<typename... Args>
void assign_next_column(MYSQL_ROW row,  int* col,  compsky::asciify::flag::guarantee::BetweenZeroAndTenLeftInclusive f,  double*& d,  Args... args){
    char* s = row[(*col)++];
    
    *d = *s - '0';
    
    ++s;
    
    if (*s == 0)
        return;
    
    ++s; // Skip the "."
    int n_digits = strlen(s);
    
    for (auto i = n_digits - 1;  i >= 0;  --i){
        *d += s[i] - '0';
        *d /= 10;
    }
    
    assign_next_column(row,  col,  args...);
};

template<typename... Args>
bool assign_next_result(MYSQL_RES* res,  MYSQL_ROW* row,  Args... args){
    if ((*row = mysql_fetch_row(res))){
      #ifdef DEBUG
        printf("Row: ");
        for (auto i = 0;  i < 5;  ++i)
            printf("%s\t", (*row)[i]);
        printf("\n");
      #endif
        int col = 0;
        assign_next_column(*row, &col, args...);
        return true;
    }
    mysql_free_result(res);
    return false;
};

} // END namespace mymysql








#endif
