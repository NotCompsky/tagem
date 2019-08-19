#ifndef TAGEM_MEDIA_RELATION_HUB_HPP
#define TAGEM_MEDIA_RELATION_HUB_HPP

#include "mainwindow.hpp"
#include <compsky/mysql/mysql.h>
#include <QDialog>


class MainWindow;


class RelationHub : public QDialog {
  public:
	RelationHub(MainWindow* const _win,  QWidget* parent = nullptr);
  private:
	void add(); // SLOT
	MainWindow* const win;
	MYSQL_RES* res;
	MYSQL_ROW row;
};


#endif
