#ifndef __COMPSKY_TAGEM_UTILS__
#define __COMPSKY_TAGEM_UTILS__

#include <inttypes.h>

#include <compsky/mysql/query.hpp>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


namespace _mysql {
	extern MYSQL* obj;
}

extern MYSQL_RES* RES1;
extern char BUF[];


inline
uint64_t get_last_insert_id(){
    uint64_t n = 0;
    compsky::mysql::query_buffer(_mysql::obj,  RES1,  "SELECT LAST_INSERT_ID() as ''");
    while (compsky::mysql::assign_next_row(RES1, &ROW1, &n));
    return n;
}

inline
uint64_t is_file_in_db(const char* const fp,  unsigned& protocol){
	constexpr static const compsky::asciify::flag::Escape _f_esc;
	compsky::mysql::query(_mysql::obj,  RES1,  BUF,
		"SELECT f.file, D.protocol, 1 "
		"FROM file_backup f "
		"JOIN dir d ON d.id=f.dir "
		"JOIN device D ON D.id=d.device "
		"WHERE CONCAT(d.name, f.name)=\"", _f_esc, '"', fp, "\" "
		"UNION "
		"SELECT f.id, D.protocol, 0 "
		"FROM file f "
		"JOIN dir d ON d.id=f.dir "
		"JOIN device D ON D.id=d.device "
		"WHERE CONCAT(d.name, f.name)=\"", _f_esc, '"', fp, "\""
	);
    
    uint64_t id = 0;
	bool is_backup;
    while(compsky::mysql::assign_next_row(RES1,  &ROW1,  &id,  &protocol, &is_backup));
    
    return id;
}


#endif
