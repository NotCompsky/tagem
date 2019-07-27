#include "mainwindow.hpp"

#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <compsky/mysql/query.hpp>

#include "name_dialog.hpp"
#include "tagchildtreeview.hpp"
#include "tagparenttreeview.hpp"


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
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

void MainWindow::add_new_tag(){
    NameDialog* tagdialog = new NameDialog("TagDialog", "TagStr");
    tagdialog->name_edit->setCompleter(this->tagcompleter);
    
    if (tagdialog->exec() != QDialog::Accepted)
        return;
    
    QString tagstr = tagdialog->name_edit->text();
    
    if (tagstr.isEmpty())
        return;
};
