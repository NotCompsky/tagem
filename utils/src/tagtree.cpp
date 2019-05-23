#include <iostream> // for std::cout, std::endl
#include <string> // for std::string
#include <vector> // for std::vector
#include "sql_utils.hpp" // for mysu::*

std::map<uint64_t, std::string> TAG2NAME;
std::map<uint64_t, std::vector<uint64_t>> TAG2CHILDREN;

constexpr const char* SEPARATOR = "  ";


void process_children(std::string& out,  uint64_t t,  int depth){
    for (auto i = 0;  i < depth;  ++i)
        out += SEPARATOR;
    out += TAG2NAME[t];
    out += "\n";
    for (uint64_t child : TAG2CHILDREN[t])
        process_children(out, child, depth+1);
}

int main(const int argc,  const char** argv){
    mysu::init(argv[1], "mytag");
    
    SQL_RES = SQL_STMT->executeQuery("SELECT id, name FROM tag");
    while (SQL_RES->next())
        TAG2NAME[SQL_RES->getUInt64(1)] = SQL_RES->getString(2);
    TAG2NAME[0] = "ROOT";
    
    SQL_RES = SQL_STMT->executeQuery("SELECT parent_id, tag_id FROM tag2parent");
    while (SQL_RES->next())
        TAG2CHILDREN[SQL_RES->getUInt64(1)].push_back(SQL_RES->getUInt64(2));
    std::string s = "";
    process_children(s, 0, 0);
    std::cout << s << std::endl;
}
