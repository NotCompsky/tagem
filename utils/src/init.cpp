#include <compsky/mysql/create_config.hpp>
#include <compsky/asciify/init.hpp>

#include <string.h> // for strlen


namespace compsky {
    namespace asciify {
        char* BUF;
        char* ITR;
    }
}


int main(){
	constexpr static const char* sql =
		#include "init.sql"
	;

	if(compsky::asciify::alloc(strlen(sql) + 1024))
		return 1;

    compsky::mysql::create_config(
        sql
        , "SELECT, INSERT, UPDATE, DELETE, EXECUTE"
        , "TAGEM_MYSQL_CFG"
    );
    
    return 0;
}
