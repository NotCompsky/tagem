#ifndef __TAGDIALOG__
#define __TAGDIALOG__

#include <QDialog>
#include <QDialogButtonBox>
#include <QString>
#include <QLineEdit>

class TagDialog : public QDialog {
    // NOTE: If vtable error, check if header in CMakeLists.txt (`add_executable(... ../include/tagdialog.hpp)`)
    Q_OBJECT
 public:
    explicit TagDialog(QString title,  QString str,  QWidget* parent = 0);
    QLineEdit* name_edit;
 private:
    QDialogButtonBox* btn_box;
};

#endif
