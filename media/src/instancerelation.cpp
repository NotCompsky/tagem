#include "instancerelation.hpp"
#include "instance_relation_dialog.hpp"


InstanceRelation::InstanceRelation(const uint64_t _id,  QPoint middle,  MainWindow* const _win,  QWidget* parent)
: id(_id)
, win(_win)
{
    this->btn = new QPushButton("Relation", parent);
    this->btn->move(middle);
    connect(this->btn, &QPushButton::clicked, this, &InstanceRelation::toggle_expand);
    this->btn->show();
};

InstanceRelation::~InstanceRelation(){
    delete this->btn;
};

void InstanceRelation::toggle_expand(){
	InstanceRelationDialog* dialog = new InstanceRelationDialog(this->id, this->win);
	dialog->exec();
	delete dialog;
};
