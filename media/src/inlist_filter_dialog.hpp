#ifndef TAGEM_MEDIA_INLIST_FILTER_DIALOG_HPP
#define TAGEM_MEDIA_INLIST_FILTER_DIALOG_HPP


#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>
#include <QRegularExpression>


struct InlistFilterRules {
	QRegularExpression filename_regexp; // Initialises to empty
	int w_min;
	int w_max;
	int h_min;
	int h_max;
	bool skip_tagged;
	
	InlistFilterRules()
	:	w_min(0)
	,	w_max(0)
	,	h_min(0)
	,	h_max(0)
	,	skip_tagged(false)
	{}
};


class InlistFilterDialog : public QDialog {
  public:
	InlistFilterDialog(QWidget* parent = nullptr);
	InlistFilterRules rules;
  private:
	void apply();
	QLineEdit* filename_regexp;
	QLineEdit* w_min;
	QLineEdit* w_max;
	QLineEdit* h_min;
	QLineEdit* h_max;
	QCheckBox* skip_tagged;
};


#endif
