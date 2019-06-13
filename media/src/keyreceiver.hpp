#ifndef __KEYRECEIVER__
#define __KEYRECEIVER__

#include <QEvent>
#include <QObject>


class MainWindow;


class KeyReceiver : public QObject{
// src: https://wiki.qt.io/How_to_catch_enter_key
    Q_OBJECT
 public:
    MainWindow* window;
 protected:
    bool eventFilter(QObject* obj,  QEvent* event);
};


#endif
