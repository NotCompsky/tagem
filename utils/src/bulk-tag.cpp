/*
Usage:
    ./bulk-tag {MYSQL_CONFIG_FILE} {{OPTIONS}} {{RELATIVE_PATHS}} - {{TAGS}}
*/

#include <string.h> // for strlen
#include <unistd.h> // for getcwd

#include <compsky/mysql/query.hpp> // for compsky::mysql::exec


namespace ERR {
    enum {
        NONE,
        UNKNOWN,
        GETCWD
    };
}

namespace compsky::asciify {
    char* BUF = (char*)malloc(4096);
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
    char cwd[4096] = "','"; // For the cncatenation later
    if (getcwd(cwd + 3,  1024) == NULL)
        return ERR::GETCWD;
    
    int i = 0;
    
    compsky::mysql::init(argv[++i]);
    
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
    
    auto f = compsky::asciify::flag::concat::start;
    auto g = compsky::asciify::flag::concat::end;
    
    const size_t cwd_len = strlen(cwd);
    cwd[cwd_len] = '/';  // Replace trailing \0
    
    auto start_prefix = compsky::asciify::flag::prefix::start;
    auto end_prefix   = compsky::asciify::flag::prefix::end;
    
    compsky::mysql::exec(
        // If tags do not already exist in table, register them
        "INSERT IGNORE into tag (name) "
        "VALUES ('",
            f, "'),('", 5,
                argv+i, argc-i,
            g,
        "')"
    );
    
    if (is_absolute){
        compsky::mysql::exec(
            "INSERT IGNORE into file2tag (tag_id, file_id) "
            "SELECT t.id, f.id "
            "FROM tag t, file f "
            "WHERE t.name IN ('",
                f, "','", 3,
                    argv+i, argc-i,
                g,
            "') AND f.name IN ('",
                f, "','", 3,
                    argv + file_argc_offset,
                    n_files,
                g,
            "')"
        );
    } else {
        compsky::mysql::exec(
            "INSERT IGNORE into file2tag (tag_id, file_id) "
            "SELECT t.id, f.id "
            "FROM tag t, file f "
            "WHERE t.name IN ('",
                f, "','", 3,
                    argv+i, argc-i,
                g,
            "') AND f.name IN ('",
                cwd + 3, // Skip prefix
                f, cwd, cwd_len,
                    argv + file_argc_offset,
                    n_files,
                g,
            "')"
        );
    }
}
