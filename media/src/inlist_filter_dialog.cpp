/*
Skip input files (from stdin) based on characteristics such as text length, number of paragraphs, file size, and image dimension.

Rules are all stored in a single struct. TODO: and therefore constitute 'profiles' which can be switched easily.
*/


#include "inlist_filter_dialog.hpp"
#include "dropdown_dialog.hpp"
#include "name_dialog.hpp"

#include <compsky/mysql/query.hpp>

#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRegExpValidator>
#include <QVBoxLayout>


namespace _mysql {
	extern MYSQL* obj;
}

extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;
extern char BUF[];

namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
}

InlistFilterDialog::InlistFilterDialog(QWidget* parent)
: QDialog(parent)
, files_from_sql__res(nullptr)
{
	QVBoxLayout* l = new QVBoxLayout(this);
	
	compsky::mysql::query_buffer(_mysql::obj,  RES1,  "SELECT name FROM settings");
	char* name;
	while (compsky::mysql::assign_next_row(RES1, &ROW1, &name)){
		this->settings_names << name;
	}
	
	{
		QHBoxLayout* hbox = new QHBoxLayout;
		{
			QPushButton* btn = new QPushButton("Load");
			connect(btn, &QPushButton::clicked, this, &InlistFilterDialog::load);
			hbox->addWidget(btn);
		}
		{
			QPushButton* btn = new QPushButton("Save");
			connect(btn, &QPushButton::clicked, this, &InlistFilterDialog::save);
			hbox->addWidget(btn);
		}
		l->addLayout(hbox);
	}
	
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
			this->files_from_which[files_from_which::directory] = new QRadioButton("Directory");
			this->files_from_which[files_from_which::url]       = new QRadioButton("Url");
			this->files_from_which[files_from_which::sql]       = new QRadioButton("SQL");
			this->files_from_which[files_from_which::bash]      = new QRadioButton("Bash");
			this->files_from_which[files_from_which::stdin]     = new QRadioButton("stdin");
			this->files_from_which[0]->setChecked(true);
			QHBoxLayout* hbox = new QHBoxLayout;
			for (auto i = 0;  i < files_from_which::COUNT;  ++i)
				hbox->addWidget(this->files_from_which[i]);
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
			table->addWidget(new QLabel("File Size"),  row,  0);
			
			this->file_sz_min = new QLineEdit("");
			this->file_sz_min->setValidator(validator_digits);
			table->addWidget(this->file_sz_min, row, 1);
			
			this->file_sz_max = new QLineEdit("");
			this->file_sz_max->setValidator(validator_digits);
			table->addWidget(this->file_sz_max, row, 2);
			
			++row;
		}
		
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
			this->start_from = new QLineEdit("SELECT id FROM file ORDER BY id DESC LIMIT 1");
			hbox->addWidget(this->start_from);
			vbox->addLayout(hbox);
		}
		{
			this->start_from_which[start_from_which::literal] = new QRadioButton("Literal");
			this->start_from_which[start_from_which::regex]   = new QRadioButton("Regex");
			this->start_from_which[start_from_which::sql]     = new QRadioButton("SQL");
			this->start_from_which[1]->setChecked(true);
			QHBoxLayout* hbox = new QHBoxLayout;
			for (auto i = 0;  i < start_from_which::COUNT;  ++i)
				hbox->addWidget(this->start_from_which[i]);
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
	this->rules.start_from = this->start_from->text();

	this->rules.skip_tagged = this->skip_tagged->isChecked();
	this->rules.skip_trans  = this->skip_trans->isChecked();
	this->rules.skip_grey   = this->skip_grey->isChecked();
	
	this->rules.files_from_which = get_checked_radio_btn_index(this->files_from_which, files_from_which::COUNT);
	this->rules.start_from_which = get_checked_radio_btn_index(this->start_from_which, start_from_which::COUNT);
	
	this->rules.file_sz_min = this->file_sz_min->text().toInt();
	this->rules.file_sz_max = this->file_sz_max->text().toInt();
	
	this->rules.w_min = this->w_min->text().toInt();
	this->rules.w_max = this->w_max->text().toInt();
	this->rules.h_min = this->h_min->text().toInt();
	this->rules.h_max = this->h_max->text().toInt();
	
	// Clear previous results
	if (this->files_from_sql__res != nullptr){
		char* _media_fp;
		while(compsky::mysql::assign_next_row(this->files_from_sql__res,  &this->files_from_sql__row,  &_media_fp));
		this->files_from_sql__res = nullptr;
	}
	
	switch(this->rules.files_from_which){
		case files_from_which::bash:
			if (this->files_from_bash.state() == QProcess::Running)
				this->files_from_bash.kill();
			this->files_from_bash.start("bash",  {"-c", this->rules.files_from});
			if (!this->files_from_bash.waitForStarted())
				// Errored
				return;
			break;
		case files_from_which::sql:
			const QStringList statements = this->rules.files_from.split(";");
			for (auto i = 0;  i < statements.size() - 1;  ++i)
				compsky::mysql::query(_mysql::obj,  this->files_from_sql__res,  BUF,  statements[i]);
			compsky::mysql::query(_mysql::obj,  this->files_from_sql__res,  BUF,  statements[statements.size()-1]);
			break;
	}
	
	this->close();
}

void InlistFilterDialog::load(){
	DropdownDialog* dialog = new DropdownDialog("Load Settings", this->settings_names);
	if (dialog->exec() != QDialog::Accepted)
		return;
	const QString name = dialog->combo_box->currentText();
	if (name.isEmpty())
		return;
	
	compsky::mysql::query(_mysql::obj,  RES1,  BUF,  "SELECT  filename_regexp, files_from, start_from, skip_tagged, skip_trans, skip_grey, files_from_which, start_from_which, file_sz_min, file_sz_max, w_max, w_min, h_max, h_min  FROM settings  WHERE name=\"", _f::esc, '"', name, "\"");
	
	char* _filename_regexp;
	char* _files_from;
	char* _start_from;
	bool _skip_tagged;
	bool _skip_trans;
	bool _skip_grey;
	unsigned int _files_from_which;
	unsigned int _start_from_which;
	char* _file_sz_min;
	char* _file_sz_max;
	char* _w_max;
	char* _w_min;
	char* _h_max;
	char* _h_min;
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &_filename_regexp, &_files_from, &_start_from, &_skip_tagged, &_skip_trans, &_skip_grey, &_files_from_which, &_start_from_which, &_file_sz_min, &_file_sz_max, &_w_max, &_w_min, &_h_max, &_h_min)){
		this->filename_regexp->setText(_filename_regexp);
		this->files_from->setText(_files_from);
		this->start_from->setText(_start_from);
		this->skip_tagged->setChecked(_skip_tagged);
		this->skip_trans->setChecked(_skip_trans);
		this->skip_grey->setChecked(_skip_grey);
		this->files_from_which[_files_from_which]->setChecked(true);
		this->start_from_which[_start_from_which]->setChecked(true);
		this->file_sz_min->setText(_file_sz_min);
		this->file_sz_max->setText(_file_sz_max);
		this->w_min->setText(_w_min);
		this->w_max->setText(_w_max);
		this->h_min->setText(_h_min);
		this->h_max->setText(_h_max);
	}
}

constexpr
static char bool2char(const bool b){
	return (b) ? '1' : '0';
}

void InlistFilterDialog::save(){
	NameDialog* dialog = new NameDialog("Save Settings As", "");
	if (dialog->exec() != QDialog::Accepted)
		return;
	const QString name = dialog->name_edit->text();
	if (name.isEmpty())
		return;
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"INSERT INTO settings (name, filename_regexp, files_from, start_from, skip_tagged, skip_trans, skip_grey, files_from_which, start_from_which, file_sz_min, file_sz_max, w_max, w_min, h_max, h_min) VALUES (",
			'"', _f::esc, '"', name, '"', ',',
			'"', _f::esc, '"', this->filename_regexp->text(), '"', ',',
			'"', _f::esc, '"', this->files_from->text(), '"', ',',
			'"', _f::esc, '"', this->start_from->text(), '"', ',',
			bool2char(this->skip_tagged->isChecked()), ',',
			bool2char(this->skip_trans->isChecked()), ',',
			bool2char(this->skip_grey->isChecked()), ',',
			get_checked_radio_btn_index(this->files_from_which, files_from_which::COUNT), ',',
			get_checked_radio_btn_index(this->start_from_which, start_from_which::COUNT), ',',
			this->file_sz_min->text().toInt(), ',',
			this->file_sz_max->text().toInt(), ',',
			this->w_max->text().toInt(), ',',
			this->w_min->text().toInt(), ',',
			this->h_max->text().toInt(), ',',
			this->h_min->text().toInt(),
		") ON DUPLICATE KEY UPDATE "
			"filename_regexp=VALUES(filename_regexp),"
			"files_from=VALUES(files_from),"
			"skip_tagged=VALUES(skip_tagged),"
			"skip_trans=VALUES(skip_trans),"
			"skip_grey=VALUES(skip_grey),"
			"files_from_which=VALUES(files_from_which),"
			"start_from_which=VALUES(start_from_which),"
			"w_max=VALUES(w_max),"
			"w_min=VALUES(w_min),"
			"h_max=VALUES(h_max),"
			"h_min=VALUES(h_min)"
	);
}
