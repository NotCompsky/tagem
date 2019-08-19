#ifndef __INSTANCERELATION__
#define __INSTANCERELATION__

#include "mainwindow.hpp"

#include <QPushButton>
#include <QStringList>


class MainWindow;


class InstanceRelation : public QObject{
    Q_OBJECT
 public:
    InstanceRelation(const uint64_t _id,  QPoint middle,  MainWindow* const _win,  QWidget* parent);
    ~InstanceRelation();
    QPushButton* btn;
    uint64_t id;
 private Q_SLOTS:
    void toggle_expand();
 private:
	MainWindow* const win;
};


#endif
