#include <iostream> // for std::cout, std::endl

#include "mymysql.hpp" // for mymysql::*
#include "mymysql_results.hpp" // for ROW, RES, COL, ERR

MYSQL_RES* RES;
MYSQL_ROW ROW;

namespace compsky::asciify {
    char* BUF = (char*)malloc(strlen("SELECT name FROM file WHERE score IS NOT NULL ORDER BY score DESC")+1);
}

int main(const int argc,  char** argv){
    mymysql::init(argv[1]);
    
    {
    char* sort;
    if (argc == 3){
        sort = argv[2];
    } else {
        sort = "DESC";
    }
    
    mymysql::query(&RES, "SELECT name FROM file WHERE score IS NOT NULL ORDER BY score ", sort);
    }
    
    
    char* s;
    while (mymysql::assign_next_result(RES, &ROW, &s))
        std::cout << s << std::endl;
}
