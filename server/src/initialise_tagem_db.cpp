/*
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
*/

#include "initialise_tagem_db.hpp"
#include <compsky/mysql/query.hpp>
#include <compsky/utils/ptrdiff.hpp>


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
		compsky::mysql::exec_buffer(mysql_obj, "UPDATE tagem_db_initialised SET version=1");
		return;
	} catch(compsky::mysql::except::SQLExec&){}
	
	const char* last_stmt = stmts;
	for (const char* itr = stmts;  *itr != 0;  ++itr){
		if (unlikely(*itr == ';')){
			compsky::mysql::exec_buffer(mysql_obj,  last_stmt,  compsky::utils::ptrdiff(itr, last_stmt));
			last_stmt = itr + 1;
		}
	}
}
