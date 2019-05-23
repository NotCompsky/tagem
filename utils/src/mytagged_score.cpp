#include <iostream> // for std::cout, std::endl
#include <string> // for std::string
#include <vector> // for std::vector
#include "sql_utils.hpp" // for mysu::*

std::map<uint64_t, std::string> TAG2NAME;
std::map<uint64_t, std::vector<uint64_t>> TAG2CHILDREN;

constexpr const int SQL__SELECT_FILE_SIZE = strlen("SELECT name FROM file WHERE score IS NOT NULL ORDER BY score desc");
char SQL__SELECT_FILE[SQL__SELECT_FILE_SIZE + 4 + 1] = "SELECT name FROM file WHERE score IS NOT NULL ORDER BY score ";


int main(const int argc,  const char** argv){
    mysu::init(argv[1], "mytag");
    
    if (argc == 3){
        memcpy(SQL__SELECT_FILE + SQL__SELECT_FILE_SIZE,  argv[2],  4);
        // NOTE: Both "asc\0" and "desc\0" have 4 characters
        SQL__SELECT_FILE[SQL__SELECT_FILE_SIZE + 4] = 0;
    }
    
    SQL_RES = SQL_STMT->executeQuery(SQL__SELECT_FILE);
    while (SQL_RES->next())
        std::cout << SQL_RES->getString(1) << std::endl;
}
