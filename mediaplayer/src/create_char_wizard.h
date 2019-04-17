#ifndef CREATE_CHAR_WINDOW_H
#define CREATE_CHAR_WINDOW_H

#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QWizard>





class Page_CreateChar : public QWizardPage{
    Q_OBJECT
  public:
    Page_CreateChar(QWidget *parent = 0);
    int nextId() const { return -1; }; // Last page returns -1
  private:
    QLineEdit* lineedit_name;
    QLineEdit* lineedit_gender_id;
    QLineEdit* lineedit_species;
    QLineEdit* lineedit_race;
    QLineEdit* lineedit_skincolour;
    QLineEdit* lineedit_haircolour;
    QLineEdit* lineedit_eyecolour;
    QLineEdit* lineedit_age;
    QLineEdit* lineedit_franchise;
    QLineEdit* lineedit_profession;
};











struct CreateCharWizardData {
    int age;
    int gender_id;
    
    char* name;
    char* species;
    char* race;
    char* skincolour;
    char* haircolour;
    char* eyecolour;
    char* franchise;
    char* profession;
};



class CreateCharWizard : public QWizard {
    Q_OBJECT
  public:
    CreateCharWizard(QWidget *parent = 0);
    
    enum { page_createchar };
    
    void accept() override;
    CreateCharWizardData* data;
};

#endif
