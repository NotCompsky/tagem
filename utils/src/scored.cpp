#include <iostream> // for std::cout, std::endl

#include <compsky/mysql/mysql.hpp> // for compsky::mysql::*
#include <compsky/mysql/query.hpp> // for ROW, RES, COL, ERR
#include <compsky/asciify/init.hpp>


MYSQL_RES* RES;
MYSQL_ROW ROW;

namespace compsky {
	namespace asciify {
		char* BUF;
		char* ITR;
	}
}

int main(const int argc,  char** argv){
	if(compsky::asciify::alloc(strlen("SELECT name FROM file WHERE score IS NOT NULL ORDER BY score DESC")+1))
		return 4;
	
    compsky::mysql::init(getenv("TAGEM_MYSQL_CFG"));
    
    {
    char* sort;
    if (argc == 2){
        sort = argv[1];
    } else {
        sort = "DESC";
    }
    
    compsky::mysql::query(&RES, "SELECT name FROM file WHERE score IS NOT NULL ORDER BY score ", sort);
    }
    
    
    char* s;
    while (compsky::mysql::assign_next_row(RES, &ROW, &s))
        std::cout << s << std::endl;
}
