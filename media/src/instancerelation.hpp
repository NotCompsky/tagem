#ifndef __INSTANCERELATION__
#define __INSTANCERELATION__

#include "mainwindow.hpp"

#include <QPushButton>
#include <QStringList>


class MainWindow;


class InstanceRelation : public QObject{
 public:
    InstanceRelation(const uint64_t _id,  QPoint middle,  MainWindow* const _win,  QWidget* parent);
    ~InstanceRelation();
    QPushButton* btn;
    uint64_t id;
 private:
	void toggle_expand(); // SLOT
	MainWindow* const win;
};


#endif
