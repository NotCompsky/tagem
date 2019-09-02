#include "box_relation.hpp"
#include "box_relation_dialog.hpp"


BoxRelation::BoxRelation(const uint64_t _id,  QPoint middle,  MainWindow* const _win,  QWidget* parent)
: id(_id)
, win(_win)
{
    this->btn = new QPushButton("Relation", parent);
    this->btn->move(middle);
    connect(this->btn, &QPushButton::clicked, this, &BoxRelation::toggle_expand);
    this->btn->show();
};

BoxRelation::~BoxRelation(){
    delete this->btn;
};

void BoxRelation::toggle_expand(){
	BoxRelationDialog* dialog = new BoxRelationDialog(this->id, this->win);
	dialog->exec();
	delete dialog;
};
