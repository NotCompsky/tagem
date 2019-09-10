#pragma once


#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>


class GetDatetime : public QDialog {
public:
	QDateTimeEdit* date_edit;
	
	GetDatetime(QWidget* parent = nullptr)
	: QDialog(parent)
	{
		QVBoxLayout* l = new QVBoxLayout;
		this->date_edit = new QDateTimeEdit(QDate::currentDate());
		this->date_edit->setDisplayFormat("yyyy.MM.dd HH.mm.ss");
		l->addWidget(this->date_edit);
		this->setLayout(l);
		
		{
			QDialogButtonBox* btn_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
			connect(btn_box, &QDialogButtonBox::accepted, this, &GetDatetime::accept);
			connect(btn_box, &QDialogButtonBox::rejected, this, &GetDatetime::reject);
			l->addWidget(btn_box);
		}
	}
};
