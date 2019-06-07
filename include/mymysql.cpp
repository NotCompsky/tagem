#include "mymysql.hpp"

#include <string.h> // for strlen
#include <stdio.h> // for fopen, fread
#include <mysql/mysql.h>
#include <sys/mman.h> // for mmap, munmap

#include "utils.hpp" // for compsky::utils::memzero_secure


/*
Format of config file must be exactly:

HOST: some.domain.name.or.localhost
PATH: /path/to/unix/socket/or/blank/if/not/using/it
USER: your_username
PWD:  your_password
DB:   your_database_name
PORT: 12345 (or 0 if not using port)
SOME_EXTRA_LINE_HERE_THAT_ISNT_PARSED_BUT_JUST_TO_KEEP_NEWLINE
*/





namespace mymysql {

constexpr const size_t AUTH_SZ = 512; // Sane max size for MySQL authorisation/config file
char* AUTH = (char*)mmap(NULL,  AUTH_SZ,  PROT_READ | PROT_WRITE,  MAP_LOCKED | MAP_PRIVATE | MAP_ANONYMOUS,  -1,  0);
char* MYSQL_AUTH[6] ; // Declared as volatile to forbid compiler from optimising overwrites away

MYSQL OBJ;



void init(const char* fp){
    FILE* f = fopen(fp, "rb");
    
    fread(AUTH, 1, AUTH_SZ, f);
    
    int n_lines = 0;
    char* itr;
    MYSQL_AUTH[0] = AUTH + 6;
    for (itr = MYSQL_AUTH[0];  n_lines < 5;  ++itr)
        if (*itr == '\n'){
            *itr = 0;
            itr += 7; // To skip "ABCD: "
            MYSQL_AUTH[++n_lines] = itr;
        }
    
    const char* host = MYSQL_AUTH[0];
    const char* path = MYSQL_AUTH[1];
    const char* user = MYSQL_AUTH[2];
    const char* pwrd = MYSQL_AUTH[3];
    const char* dbnm = MYSQL_AUTH[4];
    
    int port_n = 0;
    while (*itr != '\n'){
        port_n *= 10;
        port_n += *itr - '0'; // Integers are continuous in every realistic character encoding
        ++itr;
    }
    
    unsigned long client_flag = CLIENT_FOUND_ROWS; // Return number of matched rows rather than number of changed rows (accessed with mysql_affected_rows(&OBJ)
    
    mysql_init(&OBJ);
    
    if (!mysql_real_connect(&OBJ, host, user, pwrd, dbnm, port_n, path, client_flag)){
        fprintf(stderr, "Failed to conenct to MySQL server at %s:%s@%s:%d%s with flag %lu\n", user, pwrd, host, port_n, path, client_flag);
        abort();
    }
}

void exit(){
    mysql_close(&OBJ);
    
    compsky::utils::memzero_secure(MYSQL_AUTH[0],  MYSQL_AUTH[5] - MYSQL_AUTH[0]); // Overwrite MySQL authorisation data 
    
    munmap(AUTH, AUTH_SZ);
}




void exec_buffer(const char* s,  const size_t sz){
    if (mysql_real_query(&mymysql::OBJ, s, sz) == 0)
        return;
    fprintf(stderr, "Error executing [%lu] %.*s\n", sz, (int)sz, s);
    abort();
};

void exec_buffer(const char* s){
    exec_buffer(s, strlen(s));
};


void query_buffer(MYSQL_RES** res,  const char* s,  const size_t sz){
  #ifdef DEBUG
    printf("Query [%lu]: %.*s\n", sz, (int)sz, s);
  #endif
    if (mysql_real_query(&mymysql::OBJ, s, sz) == 0){
        *res = mysql_store_result(&mymysql::OBJ);
        return;
    }
    fprintf(stderr, "Error executing query [%lu] %.*s\n", sz, (int)sz, s);
    abort();
};

void query_buffer(MYSQL_RES** res,  const char* s){
    query_buffer(res, s, strlen(s));
};

/* Base Case */
void assign_next_column(MYSQL_ROW row,  int* col){};


namespace flag {
    const SizeOfAssigned size_of_assigned;
}

} // END namespace mymysql
