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

} // END namespace mymysql

#endif
