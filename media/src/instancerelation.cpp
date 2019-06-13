#include "instancerelation.hpp"


InstanceRelation::InstanceRelation(QPoint middle,  QWidget* parent) : is_expanded(true){
    this->btn = new QPushButton("Relation", parent);
    this->btn->move(middle);
    this->toggle_expand();
    connect(this->btn, SIGNAL(clicked()), this, SLOT(toggle_expand()));
    this->btn->show();
};

InstanceRelation::~InstanceRelation(){
    delete this->btn;
};

void InstanceRelation::toggle_expand(){
    if (this->is_expanded)
        this->btn->setText("Relation");
    else
        this->btn->setText(this->tags.join("\n"));
    this->show_text();
    this->is_expanded = !this->is_expanded;
};

void InstanceRelation::show_text(){
    this->btn->resize(QSize(this->btn->sizeHint().width(), this->btn->sizeHint().height()));
};
