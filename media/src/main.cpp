#include <QApplication>

#ifdef VID
  #include <QtAVWidgets>
#endif

#include "mainwindow.h"


int dummy_argc = 0;
char** dummy_argv;


int main(const int argc,  const char** argv){
  #ifdef VID
    QtAV::Widgets::registerRenderers();
  #endif
    QApplication app(dummy_argc, dummy_argv);
    MainWindow player(argc, argv);
    player.show();
    return app.exec();
}
