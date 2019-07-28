#include <QApplication>

#ifdef VID
  #include <QtAVWidgets>
#endif

#include <compsky/mysql/mysql.hpp> // for compsky::mysql::*, BUF, BUF_INDX

#include "mainwindow.hpp"


int dummy_argc = 0;
char** dummy_argv;


int main(const int argc,  const char** argv){
    compsky::mysql::init(getenv("TAGEM_MYSQL_CFG"));
  #ifdef VID
    QtAV::Widgets::registerRenderers();
  #endif
    QApplication app(dummy_argc, dummy_argv);
    MainWindow player;
    player.show();
    
    int rc = app.exec();
    compsky::mysql::exit_mysql();
    return rc;
}
