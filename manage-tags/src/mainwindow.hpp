#ifndef __MAINWINDOW__
#define __MAINWINDOW__

#include <QStandardItem>
#include <QStandardItemModel>
#include <QDebug> // TMP
#include <QHeaderView>
#include <QMainWindow>
#include <QMimeData>
#include <QModelIndex>
#include <QPushButton>

#include "unpackaged_utils.hpp" // for ascii_to_uint64
#include "tagtreemodel.hpp"
#include "tagtreeview.hpp"

#include "tagdialog.hpp"

class TagChildTreeView : public TagTreeView {
 public:
    TagChildTreeView(QWidget* parent);
};


class TagParentTreeView : public TagTreeView {
    Q_OBJECT
 public:
    TagParentTreeView(TagChildTreeView* tag_child_tree_view,  QWidget* parent);
    TagChildTreeView* tag_child_tree_view;
 public Q_SLOTS:
    void set_root(const QItemSelection& selected,  const QItemSelection& deselected);
};


class MainWindow : public QMainWindow {
 public:
    MainWindow(QWidget* parent = 0);
    
    QStringList tagslist;
    QCompleter* tagcompleter;
    QPushButton* commit_btn;
    
    void add_new_tag();
};

#endif
