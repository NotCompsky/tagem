#ifndef __INSTANCERELATION__
#define __INSTANCERELATION__

#include "../mainwindow.hpp"

#include <QPushButton>
#include <QStringList>


class MainWindow;


class BoxRelation : public QObject{
 public:
    BoxRelation(const uint64_t _id,  QPoint middle,  MainWindow* const _win,  QWidget* parent);
    ~BoxRelation();
    QPushButton* btn;
    uint64_t id;
 private:
	void toggle_expand(); // SLOT
	MainWindow* const win;
};


#endif
