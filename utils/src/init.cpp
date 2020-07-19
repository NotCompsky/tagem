#include <compsky/mysql/create_config.hpp>


int main(){
	constexpr static const char* sql =
		#include "init.sql"
		#include "triggers.sql"
		#include "procedures.sql"
	;

    compsky::mysql::create_config(
        sql
        , "SELECT, INSERT, UPDATE, DELETE, EXECUTE"
        , "TAGEM_MYSQL_CFG"
    );
    
    return 0;
}
