/*
Usage:
    myttag {MYSQL_CONFIG_FILE} {{OPTIONS}} {{RELATIVE_PATHS}} - {{TAGS}}
*/

#include <string.h> // for strlen, memcpy
#include <unistd.h> // for getcwd

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
    char cwd[4096];
    if (getcwd(cwd, 1024) == NULL)
        return ERR::GETCWD;
    
    int i = 0;
    
    mymysql::init(argv[++i]);
    
    bool is_absolute = false;
    
    while (i < argc){
        const char* arg = argv[++i];
        if (arg[0] != '-' || arg[2] != 0)
            break;
        
        switch(arg[1]){
            case 'a':
                is_absolute = true;
                break;
            case 's':
                score = ascii2n(argv[++i]);
                break;
        }
    }
    --i;
    
    int file_argc_offset  =  i + 1;
    
    while (i < argc){
        const char* arg = argv[++i];
        if (arg[0] == '-'  &&  arg[1] == 0)
            break;
    }
    
    ++i; // Skip delineating argument flag
    
    int n_files  =  i - file_argc_offset;
    
    // All remaining arguments (i.e. argv[j] for all i<=j<argc) are tags
    
    StartConcatWith start_concat_bckts("'),('", 5);
    EndConcatWith end_concat_brckts;
    
    StartConcatWithApostrapheAndCommaFlag start_ap;
    EndConcatWithApostrapheAndCommaFlag end_ap;
    
    auto cwd_len = strlen(cwd);
    cwd[cwd_len] = '/';  // Replace trailing \0
    StartPrefixFlag start_prefix(cwd, cwd_len);
    EndPrefixFlag end_prefix;
    
    mymysql::exec(
        // If tags do not already exist in table, register them
        "INSERT IGNORE into tag (name) "
        "VALUES ('",
            start_concat_bckts,
                argv+i, argc-i,
            end_concat_brckts,
        "')"
    );
    
    if (is_absolute){
        mymysql::exec(
            "INSERT IGNORE into file2tag (tag_id, file_id) "
            "SELECT t.id, f.id "
            "FROM tag t, file f "
            "WHERE t.name IN (",
                start_ap,
                    argv+i, argc-i ,
                end_ap,
            ") AND f.name IN (",
                start_ap,
                    argv + file_argc_offset,
                    n_files,
                end_ap,
            ")"
        );
    } else {
        mymysql::exec(
            "INSERT IGNORE into file2tag (tag_id, file_id) "
            "SELECT t.id, f.id "
            "FROM tag t, file f "
            "WHERE t.name IN (",
                start_ap,
                    argv+i, argc-i ,
                end_ap,
            ") AND f.name IN (",
                start_ap,
                    start_prefix,
                        argv + file_argc_offset,
                        n_files,
                    end_prefix,
                end_ap,
            ")"
        );
    }
}
