#ifndef TAGEM_MEDIA_INLIST_FILTER_DIALOG_HPP
#define TAGEM_MEDIA_INLIST_FILTER_DIALOG_HPP


#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QTextEdit>
#include <QProcess>
#include <QRadioButton>
#include <QRegularExpression>

#include <compsky/mysql/mysql.hpp>
#include <compsky/mysql/query.hpp>


namespace _mysql {
	extern MYSQL* obj;
}

extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;
extern char BUF[];


namespace files_from_which {
	enum {
		stdin,
		directory,
		sql,
		bash,
		url,
		COUNT
	};
}

namespace start_from_which {
	enum {
		literal,
		regex,
		sql,
		COUNT
	};
}


struct InlistFilterRules {
	QRegularExpression filename_regexp; // Initialises to empty
	QString files_from;
	QString start_from;
	size_t file_sz_min;
	size_t file_sz_max;
	int w_min;
	int w_max;
	int h_min;
	int h_max;
	unsigned int files_from_which;
	unsigned int start_from_which;
	bool skip_tagged;
	bool skip_trans;
	bool skip_grey;
	
	InlistFilterRules()
	:	file_sz_min(0)
	,	file_sz_max(0)
	,	w_min(0)
	,	w_max(0)
	,	h_min(0)
	,	h_max(0)
	,	files_from_which(files_from_which::stdin)
	,	start_from_which(start_from_which::literal)
	,	skip_tagged(false)
	{}
};


class InlistFilterDialog : public QDialog {
  public:
	InlistFilterDialog(QWidget* parent = nullptr);
	
	InlistFilterRules rules;
	
	MYSQL_RES* files_from_sql__res;
	MYSQL_ROW files_from_sql__row;
	QProcess files_from_bash;
  private:
	void apply();
	void save();
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
		constexpr static const compsky::asciify::flag::Escape f_esc;
		
		compsky::mysql::query(_mysql::obj,  RES1,  BUF,  "SELECT  filename_regexp, files_from, start_from, skip_tagged, skip_trans, skip_grey, files_from_which, start_from_which, file_sz_min, file_sz_max, w_max, w_min, h_max, h_min  FROM settings  WHERE name=\"", f_esc, '"', s, "\"");
		
		unsigned int count = 0;
		
		const char* _filename_regexp;
		const char* _files_from;
		const char* _start_from;
		bool _skip_tagged;
		bool _skip_trans;
		bool _skip_grey;
		unsigned int _files_from_which;
		unsigned int _start_from_which;
		size_t _file_sz_min;
		size_t _file_sz_max;
		int _w_max;
		int _w_min;
		int _h_max;
		int _h_min;
		while(compsky::mysql::assign_next_row(RES1, &ROW1, &_filename_regexp, &_files_from, &_start_from, &_skip_tagged, &_skip_trans, &_skip_grey, &_files_from_which, &_start_from_which, &_file_sz_min, &_file_sz_max, &_w_max, &_w_min, &_h_max, &_h_min)){
			++count;
			
			this->rules.filename_regexp.setPattern(_filename_regexp);
			this->rules.files_from = _files_from;
			this->rules.start_from = _start_from;

			this->rules.skip_tagged = _skip_tagged;
			this->rules.skip_trans  = _skip_trans;
			this->rules.skip_grey   = _skip_grey;
			
			this->rules.files_from_which = _files_from_which;
			this->rules.start_from_which = _start_from_which;
			
			this->rules.file_sz_min = _file_sz_min;
			this->rules.file_sz_max = _file_sz_max;
			
			this->rules.w_min = _w_min;
			this->rules.w_max = _w_max;
			this->rules.h_min = _h_min;
			this->rules.h_max = _h_max;
			
			this->filename_regexp->setText(_filename_regexp);
			this->files_from->setPlainText(_files_from);
			this->start_from->setText(_start_from);
			this->skip_tagged->setChecked(_skip_tagged);
			this->skip_trans->setChecked(_skip_trans);
			this->skip_grey->setChecked(_skip_grey);
			this->files_from_which[_files_from_which]->setChecked(true);
			this->start_from_which[_start_from_which]->setChecked(true);
			this->file_sz_min->setText(QString("%1").arg(_file_sz_min));
			this->file_sz_max->setText(QString("%1").arg(_file_sz_max));
			this->w_min->setText(QString("%1").arg(_w_min));
			this->w_max->setText(QString("%1").arg(_w_max));
			this->h_min->setText(QString("%1").arg(_h_min));
			this->h_max->setText(QString("%1").arg(_h_max));
		}
		
		if (count == 0)
			QMessageBox::warning(this,  "No such rule",  s);
		
		return count;
		// WARNING: The int should only ever have the values 1 and 0.
	};
	
	void load_from_textedit(){
		this->load(this->settings_name->text());
	};
};


#endif
