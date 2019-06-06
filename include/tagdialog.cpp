#include "tagdialog.hpp"

#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>


TagDialog::TagDialog(QString title,  QString str,  QWidget* parent) : QDialog(parent){
    // If the functions are implemented in the header file you have to declare the definitions of the functions with inline to prevent having multiple definitions of the functions.
    this->btn_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(this->btn_box, SIGNAL(accepted()), this, SLOT(accept()));
    connect(this->btn_box, SIGNAL(rejected()), this, SLOT(reject()));
    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(this->btn_box);
    this->name_edit = new QLineEdit(str);
    l->addWidget(this->name_edit);
    QLabel* guide = new QLabel(tr("Enter blank tag to designate as root tag"));
    l->addWidget(guide);
    this->setLayout(l);
    this->setWindowTitle(title);
    QTimer::singleShot(0, this->name_edit, SLOT(setFocus())); // Set focus after TagDialog instance is visible
};
