#ifndef __COMPSKY_MYMYSQL__
#define __COMPSKY_MYMYSQL__


#include <stdio.h> // for fopen, fread
#include <mysql/mysql.h>
#include <sys/mman.h> // for mmap, munmap

#include "utils.hpp" // for compsky::utils::*


/* Definitions used only in mymysql_results.hpp */
struct SizeOfAssigned{
    size_t size;
};



namespace mymysql {

extern const size_t AUTH_SZ;
extern char* AUTH;
extern char* MYSQL_AUTH[6];

extern MYSQL OBJ;



void init(const char* fp);

void exit();






extern MYSQL_RES* RES; // Not used in this header, only for convenience
extern MYSQL_ROW ROW; // Not used in this header, only for convenience


void exec_buffer(const char* s,  const size_t sz);
void exec_buffer(const char* s);

void query_buffer(MYSQL_RES** res,  const char* s,  const size_t sz);
void query_buffer(MYSQL_RES** res,  const char* s);

void assign_next_column(MYSQL_ROW row,  int* col);

void free_result();

namespace flag {
    struct SizeOfAssigned{};
}

} // END namespace mymysql

#endif
