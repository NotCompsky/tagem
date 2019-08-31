#include "mainwindow.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "tagparenttreeview.hpp"


TagManager::TagManager(QWidget* parent) : QDialog(parent) {
    // Inlined to avoid multiple definition error
    QVBoxLayout* vl = new QVBoxLayout();
    QHBoxLayout* hl = new QHBoxLayout();
	tag_child_tree_view = new TagChildTreeView(this);
    hl->addWidget(tag_child_tree_view);
    TagParentTreeView* tag_parent_tree_view = new TagParentTreeView(tag_child_tree_view, this);
    hl->addWidget(tag_parent_tree_view);
    vl->addLayout(hl);
	this->setLayout(vl);
}
