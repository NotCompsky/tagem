#ifndef __OVERLAY__
#define __OVERLAY__

#include <QPaintEvent>
#include <QWidget>

class MainWindow;


class Overlay : public QWidget{
    Q_OBJECT
 public:
    Overlay(MainWindow* win,  QWidget* parent);
    bool do_not_update_instances;
 protected:
    void paintEvent(QPaintEvent* e) override;
 private:
    MainWindow* win;
};


#endif
