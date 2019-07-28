#ifndef TAGEM_MEDIA_INLIST_FILTER_DIALOG_HPP
#define TAGEM_MEDIA_INLIST_FILTER_DIALOG_HPP


#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>
#include <QRegularExpression>


struct InlistFilterRules {
	QRegularExpression filename_regexp; // Initialises to empty
	bool skip_tagged;
};


class InlistFilterDialog : public QDialog {
  public:
	InlistFilterDialog(QWidget* parent = nullptr);
	InlistFilterRules rules;
  private:
	void apply();
	QLineEdit* filename_regexp;
	QCheckBox* skip_tagged;
};


#endif
