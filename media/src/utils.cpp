#include "utils.hpp"

#include <compsky/mysql/query.hpp>


namespace _f {
    constexpr static const compsky::asciify::flag::Escape esc;
}

uint64_t get_last_insert_id(){
    uint64_t n = 0;
    compsky::mysql::query_buffer(&RES1,  "SELECT LAST_INSERT_ID() as ''");
    while (compsky::mysql::assign_next_result(RES1, &ROW1, &n));
    return n;
}

uint64_t is_file_in_db(const char* fp){
    constexpr const char* a = "SELECT id FROM file WHERE name=\"";
    
    compsky::mysql::query(&RES1,  "SELECT id FROM file WHERE name=\"", _f::esc, '"', fp, "\"");
    
    uint64_t id = 0;
    while(compsky::mysql::assign_next_result(RES1,  &ROW1,  &id));
    
    return id;
}
