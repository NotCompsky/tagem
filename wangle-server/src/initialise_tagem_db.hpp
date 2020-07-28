#pragma once

#include <compsky/mysql/query.hpp>


void initialise_tagem_db(MYSQL* mysql_obj){
	constexpr static const char* const stmts =
		#include "../../utils/src/init.sql"
		#include "../../utils/src/init_data.sql"
		#include "../../utils/src/triggers.sql"
		#include "../../utils/src/procedures.sql"
		#include "../../utils/src/init_user_invalid.sql"
		#include "../../utils/src/init_user_guest.sql"
		#include "../../utils/src/init_user_admin.sql"
	;
	
	for (const char* itr = stmts;  *itr != 0;  ++itr)
		if (unlikely(*itr == ';'))
			compsky::mysql::exec_buffer(mysql_obj, stmts,  (uintptr_t)itr - (uintptr_t)stmts);
}
