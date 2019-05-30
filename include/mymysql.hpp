#ifndef __COMPSKY_MYMYSQL__
#define __COMPSKY_MYMYSQL__

#include <stdio.h> // for fopen, fread
#include <mysql/mysql.h>

#include "utils.hpp" // for asciify



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
    char* ptrs[6];
    char* itr;
    ptrs[0] = BUF + 6;
    for (itr = ptrs[0];  n_lines < 5;  ++itr)
        if (*itr == '\n'){
            *itr = 0;
            itr += 7; // To skip "ABCD: "
            ptrs[++n_lines] = itr;
        }
    
    const char* host = ptrs[0];
    const char* path = ptrs[1];
    const char* user = ptrs[2];
    const char* pwrd = ptrs[3];
    const char* dbnm = ptrs[4];
    
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
}

} // END namespace mymysql

#endif
