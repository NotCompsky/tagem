#include <QApplication>

#include <compsky/mysql/mysql.hpp>
#include <compsky/asciify/init.hpp>

#include "mainwindow.hpp"

namespace compsky {
    namespace asciify {
        char* BUF;
		char* ITR;
    }
}


MYSQL_RES* RES;
MYSQL_ROW ROW;


int main(int argc,  char** argv){
	if(compsky::asciify::alloc(4096))
		return 4;
    QApplication app(argc, argv);
    compsky::mysql::init(getenv("TAGEM_MYSQL_CFG"));
    MainWindow win;
    win.show();
    int rc = app.exec();
    compsky::mysql::exit_mysql();
    return rc;
}
