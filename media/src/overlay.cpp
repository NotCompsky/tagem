#include "overlay.hpp"

#include <QPainter>

#include "instancerelation.hpp"
#include "instancewidget.hpp"
#include "mainwindow.hpp"


Overlay::Overlay(MainWindow* win,  QWidget* parent)  :  win(win), QWidget(parent){
    this->setAttribute(Qt::WA_NoSystemBackground);
    this->setAttribute(Qt::WA_TransparentForMouseEvents);
};

void Overlay::paintEvent(QPaintEvent* e){
    if (this->do_not_update_boxes)
        return;
    QPainter painter(this);
    QPen pen;
    pen.setStyle(Qt::DashLine); // https://doc.qt.io/qt-5/qt.html#PenStyle-enum
    pen.setWidth(3);
    pen.setBrush(Qt::green);
    painter.setPen(pen);
    painter.save();
    foreach(BoxWidget* iw,  this->win->box_widgets){
        for (auto iter = iw->relations.begin();  iter != iw->relations.end();  iter++){
            // TODO: Add triangular button along this line that additionally indicates the heirarchy of the relation
            QPoint master;
            QPoint slave;
            master = iw->geometry.topRight();
            slave  = iter->first->geometry.topLeft();
            painter.drawLine(master, slave);
            QPoint p = (master + slave) / 2;
            iter->second->btn->move(p);
        }
    }
    painter.restore();
}
