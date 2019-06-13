#ifndef __COMPSKY_TAGEM_UTILS__
#define __COMPSKY_TAGEM_UTILS__

#include <inttypes.h>

#include <mysql/mysql.h>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


uint64_t get_last_insert_id();
uint64_t is_file_in_db(const char* fp);


#endif
