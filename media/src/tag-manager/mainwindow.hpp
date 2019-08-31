#ifndef __MAINWINDOW__
#define __MAINWINDOW__

#include <QCompleter>
#include <QDialog>
#include <QPushButton>
#include <QStringList>


class TagManager : public QDialog {
 public:
	TagManager(QWidget* parent = 0);
	void add_new_tag();
	
    QPushButton* commit_btn;
};

#endif
