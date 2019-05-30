#ifndef __COMPSKY_MYMYSQL__
#define __COMPSKY_MYMYSQL__

#include <stdio.h> // for fopen, fread
#include <mysql/mysql.h>

#include "utils.hpp" // for asciify, memzero_secure



extern int BUF_SZ;
extern char* BUF;
extern int BUF_INDX;

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

char* MYSQL_AUTH[6] ; // Declared as volatile to forbid compiler from optimising overwrites away

MYSQL OBJ;


void exec(const char* s){
    if (mysql_real_query(&mymysql::OBJ, s, strlen(s)) == 0)
        return;
    fprintf(stderr, "Error executing %s\n", s);
    exit(1);
};

template<typename... Args>
void exec_noclearbuf(Args... args){
    asciify(args...);
    BUF[BUF_INDX] = 0;
    if (mysql_real_query(&mymysql::OBJ, BUF, BUF_INDX) == 0)
        return;
    fprintf(stderr, "Error executing %s\n", BUF);
    exit(1);
};

template<typename... Args>
void exec(Args... args){
    exec_noclearbuf(args...);
    BUF_INDX = 0;
};



void init(const char* fp){
    FILE* f = fopen(fp, "rb");
    
    fread(BUF, 1, BUF_SZ, f);
    
    int n_lines = 0;
    char* itr;
    MYSQL_AUTH[0] = BUF + 6;
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
        fprintf(stderr, "Failed to conenct to MySQL server at %s:%s@%s:%d/%s with flag %lu\n", user, pwrd, host, port_n, path, client_flag);
        exit(1);
    }
}

void exit(){
    mysql_close(&OBJ);
    
    memzero_secure(MYSQL_AUTH[0],  MYSQL_AUTH[5] - MYSQL_AUTH[0]); // Overwrite MySQL authorisation data 
    // memset may be optimised away by compilers. This optimisation is prohibited for memset_s
    // Might only be available in C11 standard - GCC (C++) is happy with it though
}

} // END namespace mymysql

#endif
