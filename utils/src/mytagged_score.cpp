#include <iostream> // for std::cout, std::endl
#include "mymysql.hpp" // for mymysql::*

namespace res1 {
    #include "mymysql_results.hpp" // for ROW, RES, COL, ERR
}


constexpr const int SQL__SELECT_FILE_SIZE = strlen("SELECT name FROM file WHERE score IS NOT NULL ORDER BY score desc");
char SQL__SELECT_FILE[SQL__SELECT_FILE_SIZE + 4 + 1] = "SELECT name FROM file WHERE score IS NOT NULL ORDER BY score ";


int main(const int argc,  char** argv){
    mymysql::init(argv[1]);
    
    {
    char* sort;
    if (argc == 3){
        sort = argv[2];
    } else {
        sort = "DESC";
    }
    
    res1::query("SELECT name FROM file WHERE score IS NOT NULL ORDER BY score ", sort);
    }
    
    
    char* s;
    while (res1::assign_next_result(&s))
        std::cout << s << std::endl;
}
