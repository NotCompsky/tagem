#include <QDebug> // for qDebug

#define ASCII_OFFSET 48

int count_digits(int n){
    // Obviously for non-negative integers only
    int count = 0;
    while (n != 0){
        ++count;
        n /= 10;
    }
    qDebug() << "count_digits:" << +count;
    return count;
}

void itoa_nonstandard(int n, const int n_digits, char* buf){
    // Obviously not safe etc. etc. No util functions are written for safety.
    // buf should be at least as long as n_digits+1
    int i = n_digits;
    while (--i >= 0){
        buf[i] = ASCII_OFFSET + (n % 10);
        n /= 10;
    }
    buf[n_digits] = 0;
    qDebug() << "itoa_nonstandard\t" << buf;
}
