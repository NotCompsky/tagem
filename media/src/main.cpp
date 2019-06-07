#include <QApplication>

#ifdef VID
  #include <QtAVWidgets>
#endif

#include "mymysql.hpp" // for mymysql::*, BUF, BUF_INDX

#include "mainwindow.h"


int dummy_argc = 0;
char** dummy_argv;


int main(const int argc,  const char** argv){
    mymysql::init(argv[1]);
  #ifdef VID
    QtAV::Widgets::registerRenderers();
  #endif
    QApplication app(dummy_argc, dummy_argv);
    MainWindow player(argc, argv);
    player.show();
    
    int rc = app.exec();
    mymysql::exit();
    return rc;
}
