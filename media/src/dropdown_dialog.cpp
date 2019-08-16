/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "dropdown_dialog.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QTimer>
#include <QVBoxLayout>


DropdownDialog::DropdownDialog(QString title,  QStringList options,  QWidget* parent) : QDialog(parent){
    // If the functions are implemented in the header file you have to declare the definitions of the functions with inline to prevent having multiple definitions of the functions.
    QDialogButtonBox* btn_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btn_box, &QDialogButtonBox::accepted, this, &DropdownDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, this, &DropdownDialog::reject);
    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(btn_box);
    this->combo_box = new QComboBox();
	this->combo_box->addItems(options);
    l->addWidget(this->combo_box);
    this->setLayout(l);
    this->setWindowTitle(title);
    QTimer::singleShot(0, this->combo_box, SLOT(setFocus())); // Set focus after DropdownDialog instance is visible
}
