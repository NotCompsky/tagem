/*
Usage:
    myttag {MYSQL_CONFIG_FILE} {{OPTIONS}} {RELATIVE_PATH} {{TAGS}}
*/

#include <string.h> // for strlen, memcpy
#include <unistd.h> // for getcwd

#include <vector>
#include "utils.hpp"
#include "mymysql.hpp" // for mymysql::*


namespace ERR {
    enum {
        NONE,
        UNKNOWN,
        GETCWD
    };
}


int ascii2n(const char* s){
    int n = 0;
    while (*s != 0){
        n *= 10;
        n += *s - '0';
        ++s;
    }
    return n;
};


int main(const int argc, const char** argv){
    int score = 0;
    char fullpath[4096];
    if (getcwd(fullpath, 4096) == NULL)
        return ERR::GETCWD;
    size_t CWD_len = strlen(fullpath);
    fullpath[CWD_len++] = '/';
    
    int i = 0;
    
    mymysql::init(argv[++i]);
    
    while (i < argc){
        const char* arg = argv[++i];
        if (arg[0] != '-' || arg[2] != 0){
            --i;
            break;
        }
        switch(arg[1]){
            case 's':
                score = ascii2n(argv[++i]);
                break;
        }
    }
    
    // All remaining arguments (i.e. argv[j] for all i<=j<argc) are tags
    
    ++i;
    memcpy(fullpath + CWD_len,  argv[i],  strlen(argv[i]));
    fullpath[CWD_len + strlen(argv[i])] = 0;
    
    
    StartConcatWithApostrapheAndCommaFlag start_ap;
    EndConcatWithApostrapheAndCommaFlag end_ap;
    
    mymysql::exec(
        // If tags do not already exist in table, register them
        "INSERT IGNORE into tag (name) "
        "VALUES (",
            start_ap,
            argv+i+1, argc-i-1,
            end_ap,
        ")"
    );
    mymysql::exec(
        "INSERT IGNORE into file2tag (tag_id, file_id) "
        "SELECT t.id, f.id "
        "FROM tag t, file f "
        "WHERE t.name IN (",
            start_ap,
            argv+i+1, argc-i-1,
            end_ap,
        ") AND f.name IN (",
            start_ap,
            fullpath,
            end_ap,
        ")"
    );
}
