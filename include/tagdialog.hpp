#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>

class TagDialog : public QDialog{
    Q_OBJECT
 public:
    explicit TagDialog(QString title,  QString str,  QWidget* parent = 0) : QDialog(parent){
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
    QLineEdit* name_edit;
 private:
    QDialogButtonBox* btn_box;
};
