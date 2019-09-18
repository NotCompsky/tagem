#ifndef TAGEM_MEDIA_INLIST_FILTER_DIALOG_HPP
#define TAGEM_MEDIA_INLIST_FILTER_DIALOG_HPP


#include "rule.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QTextEdit>
#include <QProcess>
#include <QRadioButton>
#include <QRegularExpression>


namespace _mysql {
	extern MYSQL* obj;
}

extern char BUF[];


class InlistFilterDialog : public QDialog {
  public:
	InlistFilterDialog(QWidget* parent = nullptr);
	
	InlistFilterRules rules{_mysql::obj, BUF};
	
	MYSQL_RES* files_from_sql__res;
	MYSQL_ROW files_from_sql__row;
	QProcess files_from_bash;
  private:
	void apply();
	void save();
	void del(); // SLOT
	QLineEdit* settings_name;
	QLineEdit* filename_regexp;
	QLineEdit* file_sz_min;
	QLineEdit* file_sz_max;
	QLineEdit* w_min;
	QLineEdit* w_max;
	QLineEdit* h_min;
	QLineEdit* h_max;
	QTextEdit* files_from;
	QLineEdit* start_from;
	QStringList settings_names;
	QRadioButton* files_from_which[files_from_which::COUNT];
	QRadioButton* start_from_which[start_from_which::COUNT];
	QCheckBox* skip_tagged;
	QCheckBox* skip_trans;
	QCheckBox* skip_grey;
 public:
	void get_results();
	
	template<typename String>
	bool load(String s /*= this->settings_name->text()*/){
		auto count = this->rules.load(s);
		
		if (count == 0)
			QMessageBox::warning(this,  "No such rule",  s);
		
		this->filename_regexp->setText(this->rules.filename_regexp.pattern());
		this->files_from->setPlainText(this->rules.files_from);
		this->start_from->setText(this->rules.start_from);
		this->skip_tagged->setChecked(this->rules.skip_tagged);
		this->skip_trans->setChecked(this->rules.skip_trans);
		this->skip_grey->setChecked(this->rules.skip_grey);
		this->files_from_which[this->rules.files_from_which]->setChecked(true);
		this->start_from_which[this->rules.start_from_which]->setChecked(true);
		this->file_sz_min->setText(QString("%1").arg(this->rules.file_sz_min));
		this->file_sz_max->setText(QString("%1").arg(this->rules.file_sz_max));
		this->w_min->setText(QString("%1").arg(this->rules.w_min));
		this->w_max->setText(QString("%1").arg(this->rules.w_max));
		this->h_min->setText(QString("%1").arg(this->rules.h_min));
		this->h_max->setText(QString("%1").arg(this->rules.h_max));
		
		return count;
		// WARNING: The int should only ever have the values 1 and 0.
	};
	
	void load_from_textedit(){
		this->load(this->settings_name->text());
	};
};


#endif
