namespace compsky::asciify {

char* BUF;
int BUF_INDX;


struct Buffer {
    Buffer(const size_t sz) : sz(sz), indx(0) {
        buf = (char*)malloc(sz);
    };
    char* buf;
    size_t sz;
    size_t indx;
};


/* Headers */
template<typename... Args>
void asciify(Buffer b,  Args... args);

template<typename... Args>
void asciify(Args... args);

template<typename T>
void asciify_integer(T n);

template<typename T,  typename... Args>
void asciify(flag::FillWithLeadingZeros f,  const int min_digits,  T n,  Args... args);

template<typename T>
bool operator <(T t,  fake_type::Infinity x);

template<typename T>
bool operator >(fake_type::Infinity x,  T t);

template<typename Precision>
void asciify_subzero(double d,  Precision precision);

template<typename T,  typename P,  typename... Args>
void asciify_floaty(T d,  P precision);

template<typename T,  typename... Args>
void asciify(double d,  T precision,  Args... args);

template<typename T,  typename... Args>
void asciify(float f,  T precision,  Args... args);

template<typename T>
void asciify(flag::guarantee::BetweenZeroAndOne f,  double d,  T precision);

template<typename T,  typename... Args>
void asciify(flag::guarantee::BetweenZeroAndOne f,  double d,  T precision,  Args... args);

template<typename T,  typename... Args>
void asciify(flag::Escape f,  const char c,  T s,  Args... args);

template<typename... Args>
void asciify(flag::concat::Start f,  const char* s,  const int sz,  const uint64_t n,  Args... args);

template<typename... Args>
void asciify(flag::concat::Start f,  const char* s,  const int sz,  const char* a,  Args... args);

template<typename... Args>
void asciify(flag::concat::Start e,  const char* s,  const int sz,  flag::concat::End f,  Args... args);






/* Base Cases */
void asciify(){}

void asciify(const char* s){
    while (*s != 0){
        BUF[BUF_INDX++] = *s;
        ++s;
    }
}
void asciify(const char c){
    BUF[BUF_INDX++] = c;
}

void asciify(flag::Escape f,  const char c,  char* s){
    while(*s != 0){
        if (*s == c  ||  *s == '\\')
            BUF[BUF_INDX++] = '\\';
        BUF[BUF_INDX++] = *s;
        ++s;
    }
}


/* Base Integer Cases */
template<typename T>
void asciify_integer(T n){
    auto n_digits = compsky::utils::count_digits(n);
    auto i = n_digits;
    BUF_INDX += i;
    do {
        BUF[--BUF_INDX] = '0' + (n % 10);
        n /= 10;
    } while (n != 0);
    BUF_INDX += n_digits;
};
void asciify(uint64_t n){
    asciify_integer(n);
}
void asciify(int64_t n){
    asciify_integer(n);
}
void asciify(uint32_t n){
    asciify_integer(n);
}
void asciify(int32_t n){
    asciify_integer(n);
}
void asciify(uint16_t n){
    asciify_integer(n);
}
void asciify(int16_t n){
    asciify_integer(n);
}
void asciify(uint8_t n){
    asciify_integer(n);
}
void asciify(int8_t n){
    asciify_integer(n);
}



/* Initialise Buffer */
template<typename... Args>
void asciify(Buffer b,  Args... args){
    BUF = b.buf;
    BUF_INDX = b.indx;
    asciify(args...);
};





template<typename T,  typename... Args>
void asciify(flag::FillWithLeadingZeros f,  const int min_digits,  T n,  Args... args){
    int n_digits = compsky::utils::count_digits(n);
    for (auto i = n_digits;  i < min_digits;  ++i)
        BUF[BUF_INDX++] = '0';
    asciify(n, args...);
};

template<typename T>
bool operator <(T t,  fake_type::Infinity x){
    return true;
};

template<typename T>
bool operator >(fake_type::Infinity x,  T t){
    return false;
};

template<typename Precision>
void asciify_subzero(double d,  Precision precision){
    for (auto i = 0;  d > 0  &&  i < precision;  ++i){
        d *= 10;
        char c = d;
        asciify('0' + c);
        d -= c;
    }
};

template<typename T,  typename P,  typename... Args>
void asciify_floaty(T d,  P precision){
    if (d < 0)
        return asciify(-d, precision);
    asciify((uint64_t)d);
    d -= (uint64_t)d;
    asciify('.');
    asciify_subzero(d, precision);
};

template<typename T,  typename... Args>
void asciify(double d,  T precision,  Args... args){
    asciify_floaty(d, precision);
    asciify(args...);
};

template<typename T,  typename... Args>
void asciify(float f,  T precision,  Args... args){
    asciify_floaty(f, precision);
    asciify(args...);
};

template<typename T>
void asciify(flag::guarantee::BetweenZeroAndOne f,  double d,  T precision){
    asciify('0' + (char)d,  '.');
    asciify_subzero(d, precision);
};

template<typename T,  typename... Args>
void asciify(flag::guarantee::BetweenZeroAndOne f,  double d,  T precision,  Args... args){
    asciify(f, d, precision);
    asciify(args...);
};

template<typename T,  typename... Args>
void asciify(flag::Escape f,  const char c,  T s,  Args... args){
    asciify(f, c, s);
    asciify(args...);
};

template<typename... Args>
void asciify(flag::concat::Start f,  const char* s,  const int sz,  const uint64_t n,  Args... args){
    asciify(n);
    asciify(flag::strlen, s, sz);
    asciify(f, s, sz, args...);
};

template<typename... Args>
void asciify(flag::concat::Start f,  const char* s,  const int sz,  const char* a,  Args... args){
    asciify(a);
    asciify(flag::strlen, s, sz);
    asciify(f, s, sz, args...);
};

template<typename... Args>
void asciify(flag::concat::Start e,  const char* s,  const int sz,  flag::concat::End f,  Args... args){
    // Overrides previous (more general) template
    --BUF_INDX;
    asciify(args...);
};

} // END namespace compsky::asciify
