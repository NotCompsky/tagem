/*
Skip input files (from stdin) based on characteristics such as text length, number of paragraphs, file size, and image dimension.

Rules are all stored in a single struct. TODO: and therefore constitute 'profiles' which can be switched easily.
*/


#include "inlist_filter_dialog.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>


InlistFilterDialog::InlistFilterDialog(QWidget* parent) : QDialog(parent) {
	QGridLayout* l = new QGridLayout(this);
	
	int row = 0;
	
	l->addWidget(new QLabel("Filename regex"),  row,  0);
	this->filename_regexp = new QLineEdit("");
	l->addWidget(this->filename_regexp, row, 1);
	++row;
	
	this->skip_tagged = new QCheckBox("Skip tagged");
	l->addWidget(this->skip_tagged, row, 0);
	++row;
	
	{
		QPushButton* btn = new QPushButton("Apply");
		connect(btn, &QPushButton::clicked, this, &InlistFilterDialog::apply);
		l->addWidget(btn, row, 0);
	}
}

void InlistFilterDialog::apply(){
	this->rules.filename_regexp.setPattern(this->filename_regexp->text());
	this->rules.skip_tagged = this->skip_tagged->isChecked();
}
