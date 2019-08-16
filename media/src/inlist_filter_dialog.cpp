/*
Skip input files (from stdin) based on characteristics such as text length, number of paragraphs, file size, and image dimension.

Rules are all stored in a single struct. TODO: and therefore constitute 'profiles' which can be switched easily.
*/


#include "inlist_filter_dialog.hpp"

#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRegExpValidator>
#include <QVBoxLayout>

#ifdef DEBUG
# include <stdio.h>
#else
# define printf(...)
#endif


InlistFilterDialog::InlistFilterDialog(QWidget* parent) : QDialog(parent) {
	QVBoxLayout* l = new QVBoxLayout(this);
	
	{
		QGroupBox* group_box = new QGroupBox("Files from");
		QVBoxLayout* vbox = new QVBoxLayout;
		{
			QHBoxLayout* hbox = new QHBoxLayout;
			this->files_from = new QLineEdit("");
			hbox->addWidget(this->files_from);
			vbox->addLayout(hbox);
		}
		{
			this->files_from_which[0] = new QRadioButton("Directory");
			this->files_from_which[1] = new QRadioButton("Url");
			this->files_from_which[2] = new QRadioButton("SQL");
	#       define N_FILES_FROM_WHICH 3
			this->files_from_which[0]->setChecked(true);
			QHBoxLayout* hbox = new QHBoxLayout;
			hbox->addWidget(this->files_from_which[0]);
			hbox->addWidget(this->files_from_which[1]);
			hbox->addWidget(this->files_from_which[2]);
			hbox->addStretch(1);
			vbox->addLayout(hbox);
		}
		group_box->setLayout(vbox);
		l->addWidget(group_box);
	}
	
	{
		QGroupBox* group_box = new QGroupBox("Filename Regex");
		this->filename_regexp = new QLineEdit("");
		QHBoxLayout* box = new QHBoxLayout;
		box->addWidget(this->filename_regexp);
		box->addStretch(1);
		group_box->setLayout(box);
		l->addWidget(group_box);
	}
	
	{
		QGridLayout* table = new QGridLayout;
		int row = 0;
		
		table->addWidget(new QLabel("Min"),  row,  1);
		table->addWidget(new QLabel("Max"),  row,  2);
		++row;
		
		QRegExpValidator* validator_digits = new QRegExpValidator(QRegExp("\\d*"));
		
		{
			table->addWidget(new QLabel("Width"),  row,  0);
			
			this->w_min = new QLineEdit("");
			this->w_min->setValidator(validator_digits);
			table->addWidget(this->w_min, row, 1);
			
			this->w_max = new QLineEdit("");
			this->w_max->setValidator(validator_digits);
			table->addWidget(this->w_max, row, 2);
			
			++row;
		}
		
		{
			table->addWidget(new QLabel("Height"),  row,  0);
			
			this->h_min = new QLineEdit("");
			this->h_min->setValidator(validator_digits);
			table->addWidget(this->h_min, row, 1);
			
			this->h_max = new QLineEdit("");
			this->h_max->setValidator(validator_digits);
			table->addWidget(this->h_max, row, 2);
			
			++row;
		}
		
		l->addLayout(table);
	}
	
	{
		QGroupBox* group_box = new QGroupBox("Skip");
		this->skip_tagged = new QCheckBox("Tagged");
		this->skip_trans  = new QCheckBox("Transparent");
		this->skip_grey   = new QCheckBox("Greyscale");
		QHBoxLayout* box = new QHBoxLayout;
		box->addWidget(this->skip_tagged);
		box->addWidget(this->skip_trans);
		box->addWidget(this->skip_grey);
		box->addStretch(1);
		group_box->setLayout(box);
		l->addWidget(group_box);
	}
	
	{
		QGroupBox* group_box = new QGroupBox("Start from file path matching");
		QVBoxLayout* vbox = new QVBoxLayout;
		{
			QHBoxLayout* hbox = new QHBoxLayout;
			this->files_from = new QLineEdit("SELECT id FROM file ORDER BY id DESC LIMIT 1");
			hbox->addWidget(this->files_from);
			vbox->addLayout(hbox);
		}
		{
			this->start_from_which[0] = new QRadioButton("Regex");
			this->start_from_which[1] = new QRadioButton("SQL");
	#       define N_START_FROM_WHICH 2
			this->start_from_which[1]->setChecked(true);
			QHBoxLayout* hbox = new QHBoxLayout;
			hbox->addWidget(this->start_from_which[0]);
			hbox->addWidget(this->start_from_which[1]);
			hbox->addStretch(1);
			vbox->addLayout(hbox);
		}
		group_box->setLayout(vbox);
		l->addWidget(group_box);
	}
	
	{
		QPushButton* btn = new QPushButton("Apply");
		connect(btn, &QPushButton::clicked, this, &InlistFilterDialog::apply);
		l->addWidget(btn);
	}
}

unsigned int get_checked_radio_btn_index(QRadioButton** arr,  const unsigned int n_elements) {
	for (auto i = 0;  i < n_elements;  ++i)
		if (arr[i]->isChecked())
			return i;
	// Guaranteed to have returned
}

void InlistFilterDialog::apply(){
	this->rules.filename_regexp.setPattern(this->filename_regexp->text());
	this->rules.files_from = this->files_from->text();

	this->rules.skip_tagged = this->skip_tagged->isChecked();
	this->rules.skip_trans  = this->skip_trans->isChecked();
	this->rules.skip_grey   = this->skip_grey->isChecked();
	
	this->rules.files_from = get_checked_radio_btn_index(this->files_from_which, N_FILES_FROM_WHICH);
	this->rules.start_from = get_checked_radio_btn_index(this->start_from_which, N_START_FROM_WHICH);
	
	this->rules.w_min = this->w_min->text().toInt();
	this->rules.w_max = this->w_max->text().toInt();
	this->rules.h_min = this->h_min->text().toInt();
	this->rules.h_max = this->h_max->text().toInt();
	
	printf("%d %d\n%d %d\n", this->rules.w_min, this->rules.w_max, this->rules.h_min, this->rules.h_max);
}
