#include "mainwindow.hpp"

#include <QCompleter>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <compsky/mysql/query.hpp>

#include "../name_dialog.hpp"
#include "tagchildtreeview.hpp"
#include "tagparenttreeview.hpp"


extern QCompleter* tagcompleter;


TagManager::TagManager(QWidget* parent) : QDialog(parent) {
    // Inlined to avoid multiple definition error
    QVBoxLayout* vl = new QVBoxLayout();
    QHBoxLayout* hl = new QHBoxLayout();
    this->commit_btn = new QPushButton("Commit (unimplemented)");
    TagChildTreeView* tag_child_tree_view = new TagChildTreeView(this);
    hl->addWidget(tag_child_tree_view);
    TagParentTreeView* tag_parent_tree_view = new TagParentTreeView(tag_child_tree_view, this);
    hl->addWidget(tag_parent_tree_view);
    vl->addLayout(hl);
    vl->addWidget(this->commit_btn);
	this->setLayout(vl);
}

void TagManager::add_new_tag(){
    NameDialog* tagdialog = new NameDialog("Tag", "");
    tagdialog->name_edit->setCompleter(tagcompleter);
    
    if (tagdialog->exec() != QDialog::Accepted)
        return;
    
    QString tagstr = tagdialog->name_edit->text();
    
    if (tagstr.isEmpty())
        return;
};
