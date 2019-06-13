#ifndef __MAINWINDOW__
#define __MAINWINDOW__

#include <QCompleter>
#include <QMainWindow>
#include <QPushButton>
#include <QStringList>


class MainWindow : public QMainWindow {
 public:
    MainWindow(QWidget* parent = 0);
    
    QStringList tagslist;
    QCompleter* tagcompleter;
    QPushButton* commit_btn;
    
    void add_new_tag();
};

#endif
