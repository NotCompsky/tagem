#ifndef TAGEM_MEDIA_INLIST_FILTER_DIALOG_HPP
#define TAGEM_MEDIA_INLIST_FILTER_DIALOG_HPP


#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QProcess>
#include <QRadioButton>
#include <QRegularExpression>

#include <compsky/mysql/mysql.hpp>


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
	void load();
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
};


#endif
