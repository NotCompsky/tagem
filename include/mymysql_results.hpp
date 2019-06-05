// Assumes mysql/mysql.h, utils.hpp and mymysql.hpp are included in main scope of main program


MYSQL_ROW ROW;
int COL;
MYSQL_RES* RES;


void exec_buffer(const char* s,  const size_t sz){
    if (mysql_real_query(&mymysql::OBJ, s, sz) == 0)
        return;
    fprintf(stderr, "Error executing %s\n", s);
    exit(1);
};

void exec_buffer(const char* s){
    if (mysql_real_query(&mymysql::OBJ, s, strlen(s)) == 0)
        return;
    fprintf(stderr, "Error executing %s\n", s);
    exit(1);
};

template<typename... Args>
void exec(Args... args){
    compsky::asciify::asciify(compsky::asciify::flag::change_buffer, compsky::asciify::BUF, 0, args...);
    compsky::asciify::BUF[compsky::asciify::BUF_INDX] = 0;
  #ifdef DEBUG
    printf("%s\n", compsky::asciify::BUF);
  #endif
    exec_buffer(compsky::asciify::BUF, compsky::asciify::BUF_INDX);
};




void query_buffer(const char* s,  const size_t sz){
    COL = 0;
    if (mysql_real_query(&mymysql::OBJ, s, sz) == 0){
        RES = mysql_store_result(&mymysql::OBJ);
        return;
    }
    fprintf(stderr, "Error executing query %s\n", s);
  #ifndef __WIN32
    fprintf(stderr, "Raw dump of entire buffer:\n");
    write(2, s, sz);
  #endif
    exit(1);
};

void query_buffer(const char* s){
    COL = 0;
    if (mysql_real_query(&mymysql::OBJ, s, strlen(s)) == 0){
        RES = mysql_store_result(&mymysql::OBJ);
        return;
    }
    fprintf(stderr, "Error executing query %s\n", s);
    exit(1);
};

template<typename... Args>
void query(Args... args){
    compsky::asciify::asciify(compsky::asciify::flag::change_buffer, compsky::asciify::BUF, 0, args...);
    query_buffer(compsky::asciify::BUF, compsky::asciify::BUF_INDX);
};



/* Headers */
template<typename... Args>
void assign_next_column(compsky::asciify::flag::guarantee::BetweenZeroAndOne f,  double*& d,  Args... args);

template<typename... Args>
void assign_next_column(char**& s,  Args... args);
/*
template<typename... Args>
void assign_next_column(char*& n,  Args... args);

template<typename... Args>
void assign_next_column(unsigned char*& n,  Args... args);
*/
template<typename... Args>
void assign_next_column(int8_t*& n,  Args... args);

template<typename... Args>
void assign_next_column(uint8_t*& n,  Args... args);

template<typename... Args>
void assign_next_column(int16_t*& n,  Args... args);

template<typename... Args>
void assign_next_column(uint16_t*& n,  Args... args);

template<typename... Args>
void assign_next_column(int32_t*& n,  Args... args);

template<typename... Args>
void assign_next_column(uint32_t*& n,  Args... args);

template<typename... Args>
void assign_next_column(int64_t*& n,  Args... args);

template<typename... Args>
void assign_next_column(uint64_t*& n,  Args... args);


/* Base Case */
void assign_next_column(){};





template<typename T>
T ascii2n(T m){
    T n = 0;
    char* s = ROW[COL++];
    while (*s != 0){
        n *= 10;
        n += *s - '0';
        ++s;
    }
    return n;
};


namespace flag {
    struct SizeOfAssigned{};
    const SizeOfAssigned size_of_assigned;
}

template<typename... Args>
void assign_next_column(flag::SizeOfAssigned*& f,  size_t*& sz,  Args... args){
    *sz = ROW[COL+1] - ROW[COL];
    return assign_next_column(args...);
};

template<typename... Args>
void assign_next_column(uint64_t*& n,  Args... args){
    *n = ascii2n(*n);
    assign_next_column(args...);
};
template<typename... Args>
void assign_next_column(int64_t*& n,  Args... args){
    *n = ascii2n(*n);
    assign_next_column(args...);
};
template<typename... Args>
void assign_next_column(uint32_t*& n,  Args... args){
    *n = ascii2n(*n);
    assign_next_column(args...);
};
template<typename... Args>
void assign_next_column(int32_t*& n,  Args... args){
    *n = ascii2n(*n);
    assign_next_column(args...);
};
template<typename... Args>
void assign_next_column(uint16_t*& n,  Args... args){
    *n = ascii2n(*n);
    assign_next_column(args...);
};
template<typename... Args>
void assign_next_column(int16_t*& n,  Args... args){
    *n = ascii2n(*n);
    assign_next_column(args...);
};
template<typename... Args>
void assign_next_column(uint8_t*& n,  Args... args){
    *n = ascii2n(*n);
    assign_next_column(args...);
};
template<typename... Args>
void assign_next_column(int8_t*& n,  Args... args){
    *n = ascii2n(*n);
    assign_next_column(args...);
};

template<typename... Args>
void assign_next_column(char**& s,  Args... args){
    *s = ROW[COL++];
    assign_next_column(args...);
};

template<typename... Args>
void assign_next_column(compsky::asciify::flag::guarantee::BetweenZeroAndOne f,  double*& d,  Args... args){
    char* s = ROW[COL++];
    ++s; // Skip the "0"
    
    if (*s == 0)
        return;
    
    ++s; // Skip the "."
    int n_digits = strlen(s);
    
    for (auto i = n_digits - 1;  i >= 0;  --i){
        *d += s[i] - '0';
        *d /= 10;
    }
    
    assign_next_column(args...);
};


void free_result(){
    mysql_free_result(RES);
}

template<typename... Args>
bool assign_next_result(Args... args){
    if ((ROW = mysql_fetch_row(RES))){
        COL = 0;
        assign_next_column(args...);
        return true;
    }
    return false;
};
