#include <QApplication>
#include <QCompleter>
#include <QStringList>

#include <compsky/mysql/mysql.hpp>

#include "mainwindow.hpp"


namespace _mysql {
	MYSQL* obj;
	constexpr static const size_t auth_sz = 512;
	char auth[auth_sz];
	MYSQL_RES* res;
	MYSQL_ROW row;
}

QCompleter* tagcompleter;
QStringList tagslist;


int main(int argc,  char** argv){
    QApplication app(argc, argv);
	compsky::mysql::init(_mysql::obj, _mysql::auth, _mysql::auth_sz, getenv("TAGEM_MYSQL_CFG"));
    MainWindow win;
    win.show();
	const int rc = app.exec();
	compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
    return rc;
}
