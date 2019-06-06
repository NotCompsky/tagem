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
    
    void add_new_tag(){
        TagDialog* tagdialog = new TagDialog("TagDialog", "TagStr");
        tagdialog->name_edit->setCompleter(this->tagcompleter);
        
        if (tagdialog->exec() != QDialog::Accepted)
            return;
        
        QString tagstr = tagdialog->name_edit->text();
        
        if (tagstr.isEmpty())
            return;
        
        qDebug() << "added new tag: " << tagstr;
    };
};

inline MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Inlined to avoid multiple definition error
    this->setWindowTitle("MyTag Tag Manager");
    QWidget* w = new QWidget();
    QVBoxLayout* vl = new QVBoxLayout();
    QHBoxLayout* hl = new QHBoxLayout();
    this->commit_btn = new QPushButton("Commit (unimplemented)");
    TagChildTreeView* tag_child_tree_view = new TagChildTreeView(this);
    hl->addWidget(tag_child_tree_view);
    TagParentTreeView* tag_parent_tree_view = new TagParentTreeView(tag_child_tree_view, this);
    hl->addWidget(tag_parent_tree_view);
    vl->addLayout(hl);
    vl->addWidget(this->commit_btn);
    w->setLayout(vl);
    this->setCentralWidget(w);
    this->show();
    
    this->add_new_tag();
}

#endif
