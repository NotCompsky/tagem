#ifndef TAGEM_MEDIA_INSTANCE_RELATION_DIALOG_HPP
#define TAGEM_MEDIA_INSTANCE_RELATION_DIALOG_HPP

#include "mainwindow.hpp"
#include <QDialog>


class InstanceRelationDialog : public QDialog {
  public:
	InstanceRelationDialog(const uint64_t _id,  MainWindow* const _win,  QWidget* parent = nullptr);
  private:
	void add_tag();
	void display_hub();
	MainWindow* const win;
	const uint64_t id;
};


#endif
