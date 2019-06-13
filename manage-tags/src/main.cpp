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


int main(int argc,  char** argv){
    QApplication app(argc, argv);
    compsky::mysql::init(getenv("TAGEM_MYSQL_CFG"));
    MainWindow win;
    win.show();
    int rc = app.exec();
    compsky::mysql::exit();
    return rc;
}
