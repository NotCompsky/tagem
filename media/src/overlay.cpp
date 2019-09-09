#include "overlay.hpp"
#ifdef BOXABLE
# include "boxes/box_relation.hpp"
# include "boxes/box_widget.hpp"
#endif
#include "mainwindow.hpp"
#include <QPainter>


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
# ifdef BOXABLE
	if (this->win->are_relations_visible){
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
	}
# endif
# ifdef ERA
	pen.setStyle(Qt::SolidLine); // https://doc.qt.io/qt-5/qt.html#PenStyle-enum
	pen.setWidth(2);
	pen.setBrush(Qt::yellow);
	painter.setPen(pen);
	painter.save();
	
	const int w = this->win->sizey_obj().width();
	const int h = this->win->sizey_obj().height();
	const int draw_at_h = h - 10;
	const double duration = this->win->m_player->duration();
	if (this->win->era_start != 0){
		// We have selected one end of an Era, and the next press of 'e' will finalise that Era.
		painter.drawEllipse(w * this->win->era_start / duration,  draw_at_h,  4,  4);
	}
	
	if (this->win->are_eras_visible){
		for (const MainWindow::Era era  :  this->win->eras){
			const int draw_at_w_1 = w * era.start / duration;
			const int draw_at_w_2 = w * era.end / duration;
			painter.drawLine(
				draw_at_w_1,
				draw_at_h,
				draw_at_w_2,
				draw_at_h
			);
		}
	}
# endif
    painter.restore();
}
