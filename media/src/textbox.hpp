#pragma once


#include <QLayout>
#include <QPlainTextEdit>
#include <QWidget>


class Textbox : public QWidget {
private:
	QPlainTextEdit* t;
public:
	Textbox(QWidget* parent = nullptr);
	void set_text(const char* const s);
};
