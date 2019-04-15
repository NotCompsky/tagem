#include <QApplication>

#include "movieplayer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MoviePlayer player;
    player.show();
    player.show();
    return app.exec();
}
