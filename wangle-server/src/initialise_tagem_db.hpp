#pragma once

#include <compsky/mysql/query.hpp>


void initialise_tagem_db(MYSQL* mysql_obj){
	constexpr static const char* const stmts =
		#include "../../utils/src/init.sql"
		#include "../../utils/src/init_user_invalid.sql"
		#include "../../utils/src/init_user_guest.sql"
		#include "../../utils/src/init_user_admin.sql"
		#include "../../utils/src/init_data.sql"
		#include "../../utils/src/triggers.sql"
		#include "../../utils/src/procedures.sql"
	;
	
	try {
		compsky::mysql::exec_buffer(mysql_obj, "CALL tagem_db_initialised()");
	} catch(compsky::mysql::except::SQLExec&){}
	
	const char* last_stmt = stmts;
	for (const char* itr = stmts;  *itr != 0;  ++itr){
		if (unlikely(*itr == ';')){
			compsky::mysql::exec_buffer(mysql_obj,  last_stmt,  (uintptr_t)itr - (uintptr_t)last_stmt);
			last_stmt = itr + 1;
		}
	}
	
	compsky::mysql::exec_buffer(mysql_obj, "delimiter ;");
}
