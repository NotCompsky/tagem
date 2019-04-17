#include "create_char_wizard.h"


Page_CreateChar::Page_CreateChar(QWidget *parent) : QWizardPage(parent){
    setTitle(tr("Fixed values"));

    QVBoxLayout* layout = new QVBoxLayout;
    
    lineedit_name       = new QLineEdit;
    lineedit_species    = new QLineEdit;
    lineedit_race       = new QLineEdit;
    lineedit_skincolour = new QLineEdit;
    lineedit_haircolour = new QLineEdit;
    lineedit_eyecolour  = new QLineEdit;
    lineedit_age        = new QLineEdit;
    lineedit_franchise  = new QLineEdit;
    lineedit_profession = new QLineEdit;
    
    registerField("name*",      lineedit_name);
    registerField("gender_id*", lineedit_gender_id);
    registerField("species*",   lineedit_species);
    registerField("race*",      lineedit_race);
    registerField("skincolour", lineedit_skincolour);
    registerField("haircolour", lineedit_haircolour);
    registerField("eyecolour",  lineedit_eyecolour);
    registerField("age",        lineedit_age);
    registerField("franchise",  lineedit_franchise);
    registerField("profession", lineedit_profession);
    
    setLayout(layout);
}







CreateCharWizard::CreateCharWizard(QWidget *parent) : QWizard(parent){
    setPage(page_createchar, new Page_CreateChar);

    setStartId(page_createchar);

  #ifndef Q_OS_MAC
    setWizardStyle(ModernStyle);
  #endif

    setWindowTitle(tr("Character Creation"));
}

void CreateCharWizard::accept(){
    char* name        = field("name").toString().toLocal8Bit().data();
    int gender_id     = field("gender_id").toInt();
    /*
    1   Vagina
    2   Penis/Ovipositor
    4   Breasts
    
    So e.g.
        0   Genderless
        1   Bussy
        2   Male
        5   Female
        6   Futa
    */
    // 1=F, 2=M  // Bits - so 0=Genderless, 3=Hermaphodite
    char* species     = field("species").toString().toLocal8Bit().data();
    char* race        = field("race").toString().toLocal8Bit().data();
    char* skincolour  = field("skincolour").toString().toLocal8Bit().data();
    char* haircolour  = field("haircolour").toString().toLocal8Bit().data();
    char* eyecolour   = field("eyecolour").toString().toLocal8Bit().data();
    int age           = field("age").toInt();
    char* franchise   = field("franchise").toString().toLocal8Bit().data();
    char* profession  = field("profession").toString().toLocal8Bit().data();
    
    data->age           = age;
    data->gender_id     = gender_id;
    data->name          = name;
    data->species       = species;
    data->race          = race;
    data->skincolour    = skincolour;
    data->haircolour    = haircolour;
    data->eyecolour     = eyecolour;
    data->franchise     = franchise;
    data->profession    = profession;
    
    QDialog::accept();
}
