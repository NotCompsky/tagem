#include <QApplication>
#include <QtAVWidgets>

#include "mainwindow.h"


int dummy_argc = 0;
char** dummy_argv;


int main(const int argc,  const char** argv){
    QtAV::Widgets::registerRenderers();
    QApplication app(dummy_argc, dummy_argv);
    MainWindow player(argc, argv);
    player.show();
    return app.exec();
}
