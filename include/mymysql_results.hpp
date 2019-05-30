// Assumes mysql/mysql.h, utils.hpp and mymysql.hpp are included in main scope of main program


MYSQL_ROW ROW;
int COL;
MYSQL_RES* RES;


template<typename... Args>
void query_noclearbuf(Args... args){
    COL = 0;
    asciify(args...);
    BUF[BUF_INDX] = 0;
    if (mysql_real_query(&mymysql::OBJ, BUF, BUF_INDX) == 0){
        RES = mysql_store_result(&mymysql::OBJ);
        return;
    }
    fprintf(stderr, "Error executing query %s\n", BUF);
    exit(1);
};

template<typename... Args>
void query(Args... args){
    query_noclearbuf(args...);
    BUF_INDX = 0;
};



/* Headers */
template<typename... Args>
void assign_next_column(DoubleBetweenZeroAndOne*& dd,  Args... args);





template<typename T>
T assign_next_column__integer(T m){
    T n = 0;
    char* s = ROW[COL++];
    while (*s != 0){
        n *= 10;
        n += *s - '0';
        ++s;
    }
    return n;
};

void assign_next_column(uint64_t*& n){
    *n = assign_next_column__integer(*n);
}
void assign_next_column(char**& s){
    *s = ROW[COL++];
}
void assign_next_column(DoubleBetweenZeroAndOne*& dd){
    (*dd).value = 0;
    
    char* s = ROW[COL++];
    ++s; // Skip the "0"
    
    if (*s == 0)
        return;
    
    ++s; // Skip the "."
    int n_digits = strlen(s);
    
    for (auto i = n_digits - 1;  i >= 0;  --i){
        (*dd).value += s[i] - '0';
        (*dd).value /= 10;
    }
}
/*
void assign_next_column(int32_t& n){
    printf("int32_t %s\n", ROW[COL++]);
};

void assign_next_column(char*& s){
    printf("char* %s\n", ROW[COL++]);
};
*/

template<typename... Args>
void assign_next_column(uint64_t*& n,  Args... args){
    *n = assign_next_column__integer(*n);
    assign_next_column(args...);
};
/*
template<typename... Args>
void assign_next_column(int32_t& n,  Args... args){
    printf("int32_t %s\n", ROW[COL++]);
    assign_next_column(args...);
};
*/

template<typename... Args>
void assign_next_column(char**& s,  Args... args){
    *s = ROW[COL++];
    assign_next_column(args...);
};

template<typename... Args>
void assign_next_column(DoubleBetweenZeroAndOne*& dd,  Args... args){
    assign_next_column(dd);
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
