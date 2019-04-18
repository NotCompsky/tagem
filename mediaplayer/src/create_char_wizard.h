#ifndef CREATE_CHAR_WINDOW_H
#define CREATE_CHAR_WINDOW_H

#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QWizard>
#include <QTabWidget>
#include <QDialogButtonBox>


struct Character{
    char* name;
    int sex_id;
    char* species;
    char* race;
    int eyecolour;
    
    int skincolour;
    int haircolour;
    int thickness;
    int height;
    int age;
    
    char* franchise;
    char* profession;
    char* nationality;
    
    int attract_to_gender;
    int attract_to_species;
    int attract_to_race;
};

class CharCreationInvariantsTab : public QWidget{
    Q_OBJECT
  public:
    explicit CharCreationInvariantsTab(QWidget* parent = 0);
    QLineEdit* nameEdit;
    QLineEdit* sexIDEdit;
    QLineEdit* speciesEdit;
    QLineEdit* raceEdit;
    QLineEdit* eyeColourEdit;
};

class CharCreationVariantsTab : public QWidget{
    Q_OBJECT
  public:
    explicit CharCreationVariantsTab(QWidget* parent = 0);
    QLineEdit* skinColourEdit; // TODO: Colour picker
    QLineEdit* hairColourEdit; // TODO: Colour picker
    QLineEdit* thicknessEdit;
    QLineEdit* heightEdit;
    QLineEdit* ageEdit;
};

class CharCreationRelationsTab : public QWidget{
    Q_OBJECT
  public:
    explicit CharCreationRelationsTab(QWidget* parent = 0);
    QLineEdit* franchiseEdit;
    QLineEdit* professionEdit;
    QLineEdit* nationalityEdit;
};

class CharCreationAttractionsTab : public QWidget{
    Q_OBJECT
  public:
    explicit CharCreationAttractionsTab(QWidget* parent = 0);
    QLineEdit* attractToGenderEdit;
    QLineEdit* attractToSpeciesEdit;
    QLineEdit* attractToRaceEdit;
};

class CharCreationDialog : public QDialog{
    Q_OBJECT
  public:
    explicit CharCreationDialog(QWidget* parent = 0);
    Character get_data() const;
  private:
    QTabWidget* tabWidget;
    QDialogButtonBox* buttonBox;
    CharCreationInvariantsTab* invariantsTab;
    CharCreationVariantsTab* variantsTab;
    CharCreationRelationsTab* relationsTab;
    CharCreationAttractionsTab* attractionsTab;
};
#endif
