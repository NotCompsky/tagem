#include <compsky/mysql/create_config.hpp>


int main(){
    compsky::mysql::create_config(
        #include "init.sql"
        , "TAGEM_MSQL_CFG"
    );
    
    return 0;
}
