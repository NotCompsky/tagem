/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef DROPDOWN_DIALOG_HPP
#define DROPDOWN_DIALOG_HPP

#include <QComboBox>
#include <QDialog>


class DropdownDialog : public QDialog {
  public:
    explicit DropdownDialog(QString title,  QStringList options,  QWidget* parent = 0);
    QComboBox* combo_box;
};


#endif
