#include "textbox.hpp"

#include <QVBoxLayout>


Textbox::Textbox(QWidget* parent)
: QWidget(parent)
{
	QVBoxLayout* l = new QVBoxLayout;
	this->t = new QPlainTextEdit;
	this->t->setReadOnly(true);
	this->t->setStyleSheet("background-color: rgba(0,0,0,0)"); // Make transparent
	l->addWidget(this->t);
	this->setLayout(l);
}

void Textbox::set_text(const char* const s){
	this->t->setPlainText(s);
	this->resize(this->sizeHint());
};
