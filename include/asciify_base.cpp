#include "asciify_base.hpp"

#include <string.h> // for memcpy

#include "asciify_core.hpp"


namespace compsky::asciify {

/* Base Cases */
void asciify(){}

void asciify(const char* s){
    for (auto i = 0;  s[i] != 0;  ++i){
        BUF[BUF_INDX++] = s[i];
    }
}

void asciify(const char c){
    BUF[BUF_INDX++] = c;
}

void asciify(flag::StrLen f,  const char* s,  const int sz){
    memcpy(BUF + BUF_INDX,  s,  sz);
    BUF_INDX += sz;
};


#ifdef QT_GUI_LIB
void asciify(const QString& qs){
    QByteArray bs = qs.toLocal8Bit();
    const char* s = bs.data();
    asciify(s);
}
#endif

}
