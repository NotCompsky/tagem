/*
Skip input files (from stdin) based on characteristics such as text length, number of paragraphs, file size, and image dimension.

Rules are all stored in a single struct. TODO: and therefore constitute 'profiles' which can be switched easily.
*/


#include "inlist_filter_dialog.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRegExpValidator>

#ifdef DEBUG
# include <stdio.h>
#else
# define printf(...)
#endif


InlistFilterDialog::InlistFilterDialog(QWidget* parent) : QDialog(parent) {
	QGridLayout* l = new QGridLayout(this);
	
	int row = 0;
	
	l->addWidget(new QLabel("Filename regex"),  row,  0);
	this->filename_regexp = new QLineEdit("");
	l->addWidget(this->filename_regexp, row, 1);
	++row;
	
	l->addWidget(new QLabel("Min"),  row,  1);
	l->addWidget(new QLabel("Max"),  row,  2);
	++row;
	
	QRegExpValidator* validator_digits = new QRegExpValidator(QRegExp("\\d*"));
	
	{
		l->addWidget(new QLabel("Width"),  row,  0);
		
		this->w_min = new QLineEdit("");
		this->w_min->setValidator(validator_digits);
		l->addWidget(this->w_min, row, 1);
		
		this->w_max = new QLineEdit("");
		this->w_max->setValidator(validator_digits);
		l->addWidget(this->w_max, row, 2);
		
		++row;
	}
	
	{
		l->addWidget(new QLabel("Height"),  row,  0);
		
		this->h_min = new QLineEdit("");
		this->h_min->setValidator(validator_digits);
		l->addWidget(this->h_min, row, 1);
		
		this->h_max = new QLineEdit("");
		this->h_max->setValidator(validator_digits);
		l->addWidget(this->h_max, row, 2);
		
		++row;
	}
	
	this->skip_tagged = new QCheckBox("Skip tagged");
	l->addWidget(this->skip_tagged, row, 0);
	++row;
	
	{
		QPushButton* btn = new QPushButton("Apply");
		connect(btn, &QPushButton::clicked, this, &InlistFilterDialog::apply);
		l->addWidget(btn, row, 0);
		++row;
	}
}

void InlistFilterDialog::apply(){
	this->rules.filename_regexp.setPattern(this->filename_regexp->text());
	this->rules.skip_tagged = this->skip_tagged->isChecked();
	
	this->rules.w_min = this->w_min->text().toInt();
	this->rules.w_max = this->w_max->text().toInt();
	this->rules.h_min = this->h_min->text().toInt();
	this->rules.h_max = this->h_max->text().toInt();
	
	printf("%d %d\n%d %d\n", this->rules.w_min, this->rules.w_max, this->rules.h_min, this->rules.h_max);
}
