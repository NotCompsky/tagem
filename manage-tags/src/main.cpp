#include <QApplication>

#include <compsky/mysql/mysql.hpp>

#include "mainwindow.hpp"

namespace compsky {
    namespace asciify {
        char* BUF = (char*)malloc(4096);
    }
}


MYSQL_RES* RES;
MYSQL_ROW ROW;

int dummy_argc = 0;
char** dummy_argv;


int main(const int argc,  const char** argv){
    QApplication app(dummy_argc, dummy_argv);
    compsky::mysql::init(getenv("TAGEM_MYSQL_CFG"));
    MainWindow win;
    win.show();
    int rc = app.exec();
    compsky::mysql::exit();
    return rc;
}
