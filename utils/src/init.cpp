#include <compsky/mysql/create_config.hpp>
#include <cstdlib> // for malloc
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
		#include "triggers.sql"
		#include "procedures.sql"
	;

	void* dummy = malloc(strlen(sql) + 1024);
	if (dummy == nullptr)
		return 1;

    compsky::mysql::create_config(
        sql
        , "SELECT, INSERT, UPDATE, DELETE, EXECUTE"
        , "TAGEM_MYSQL_CFG"
    );
    
    return 0;
}
