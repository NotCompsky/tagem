#include <QApplication>

#include "mymysql.hpp" // for mymysql::*, BUF, BUF_INDX

#include "mainwindow.hpp"

namespace compsky::asciify {
    char* BUF = (char*)malloc(4096);
}

namespace detail {
    // For primaryitem.hpp
    char* BUF = (char*)malloc(4096);
    size_t BUF_SZ = 4096;
    size_t BUF_INDX = 4096;
    
    void enlarge_buf(){
        BUF_INDX += BUF_SZ;
        BUF = (char*)realloc(BUF, BUF_SZ*=2);
    }
}

int dummy_argc = 0;
char** dummy_argv;


int main(const int argc,  const char** argv){
    QApplication app(dummy_argc, dummy_argv);
    mymysql::init(argv[1]);
    MainWindow win;
    win.show();
    int rc = app.exec();
    mymysql::exit();
    return rc;
}
