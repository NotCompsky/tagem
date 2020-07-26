#include <compsky/mysql/create_config.hpp>


int main(){
	constexpr static const char* sql =
		#include "init.sql"
		#include "init_data.sql"
		#include "triggers.sql"
		#include "procedures.sql"
		#include "init_user_invalid.sql"
		#include "init_user_guest.sql"
		#include "init_user_admin.sql"
	;

    compsky::mysql::create_config(
        sql
        , "SELECT, INSERT, UPDATE, DELETE, EXECUTE"
        , "TAGEM_MYSQL_CFG"
    );
    
    return 0;
}
