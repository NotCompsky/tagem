/*
Usage:
    myttag {MYSQL_CONFIG_FILE} {{OPTIONS}} {RELATIVE_PATH} {{TAGS}}
*/

#include "sql_utils.hpp" // for mysu::*, SQL_*

#include <cstdlib> // for atoi
#include <string.h> // for strlen, memcpy
#include <unistd.h> // for getcwd

#include "utils.h" // for count_digits, sql__file_attr_id


namespace ERR {
    enum {
        NONE,
        UNKNOWN,
        GETCWD
    };
}

char STMT[4096];


int main(const int argc, const char** argv){
    int score = 0;
    char fullpath[4096];
    if (getcwd(fullpath, 4096) == NULL)
        return ERR::GETCWD;
    size_t CWD_len = strlen(fullpath);
    fullpath[CWD_len++] = '/';
    
    int i = 0;
    
    mysu::init(argv[++i], "mytag");
    
    while (i < argc){
        const char* arg = argv[++i];
        if (arg[0] != '-' || arg[2] != 0){
            --i;
            break;
        }
        switch(arg[1]){
            case 's':
                // Score
                score = atoi(argv[++i]);
                break;
        }
    }
    
    // All remaining arguments (i.e. argv[j] for all i<=j<argc) are tags
    
    ++i;
    memcpy(fullpath + CWD_len,  argv[i],  strlen(argv[i]));
    fullpath[CWD_len + strlen(argv[i])] = 0;
    
    uint64_t file_id;
    file_id = sql__get_id_from_table(SQL_STMT, SQL_RES, "file", fullpath, file_id);
    auto file_id_str_len = count_digits(file_id);
    char file_id_str[file_id_str_len + 1];
    itoa_nonstandard(file_id, file_id_str_len, file_id_str);
    file_id_str[file_id_str_len] = 0;
    
    while (++i < argc){
        uint64_t tag_id;
        tag_id = sql__get_id_from_table(SQL_STMT, SQL_RES, "tag", argv[i], tag_id);
        sql__file_attr_id(SQL_STMT, SQL_RES, "tag", tag_id, file_id_str, file_id_str_len);
    }
}
