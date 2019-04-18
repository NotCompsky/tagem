#include "create_char_wizard.h"
#include <cstring> // for stoi
#include <QGroupBox>
#include <QComboBox>
#include <QDebug>


CharCreationDialog::CharCreationDialog(QWidget* parent){
    tabWidget = new QTabWidget;
    invariantsTab = new CharCreationInvariantsTab();
    variantsTab = new CharCreationVariantsTab();
    relationsTab = new CharCreationRelationsTab();
    attractionsTab = new CharCreationAttractionsTab();
    tabWidget->addTab(invariantsTab, tr("Invariants"));
    tabWidget->addTab(variantsTab, tr("Variants"));
    tabWidget->addTab(relationsTab, tr("Relations"));
    tabWidget->addTab(attractionsTab, tr("Attractions"));
    
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    
    setWindowTitle(tr("CharCreation Dialog"));
}

CharCreationInvariantsTab::CharCreationInvariantsTab(QWidget* parent) : QWidget(parent){
    QLabel* nameLabel = new QLabel(tr("Name"));
    nameEdit = new QLineEdit("");

    QLabel* sexIDLabel = new QLabel(tr("SexID"));
    sexIDEdit = new QLineEdit("2");
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
    
    QLabel* speciesLabel = new QLabel(tr("Species"));
    speciesEdit = new QLineEdit("Human");
    
    QLabel* raceLabel = new QLabel(tr("Race"));
    raceEdit = new QLineEdit("White");
    
    QLabel* eyeColourLabel = new QLabel(tr("EyeColour"));
    eyeColourEdit = new QLineEdit("Brown");
    
    
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(nameEdit);
    mainLayout->addWidget(sexIDLabel);
    mainLayout->addWidget(sexIDEdit);
    mainLayout->addWidget(speciesLabel);
    mainLayout->addWidget(speciesEdit);
    mainLayout->addWidget(raceLabel);
    mainLayout->addWidget(raceEdit);
    mainLayout->addWidget(eyeColourLabel);
    mainLayout->addWidget(eyeColourEdit);
    
    setLayout(mainLayout);
}

CharCreationVariantsTab::CharCreationVariantsTab(QWidget* parent) : QWidget(parent){
    QLabel* skinColourLabel = new QLabel(tr("SkinColour"));
    skinColourEdit = new QLineEdit(""); // TODO: Colour picker

    QLabel* hairColourLabel = new QLabel(tr("HairColour"));
    hairColourEdit = new QLineEdit("Brown"); // TODO: Colour picker
    
    QLabel* thicknessLabel = new QLabel(tr("Thickness"));
    thicknessEdit = new QLineEdit("");
    
    QLabel* heightLabel = new QLabel(tr("Height"));
    heightEdit = new QLineEdit("1.8");
    
    QLabel* ageLabel = new QLabel(tr("Age"));
    ageEdit = new QLineEdit("20");

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(skinColourLabel);
    mainLayout->addWidget(skinColourEdit);
    mainLayout->addWidget(hairColourLabel);
    mainLayout->addWidget(hairColourEdit);
    mainLayout->addWidget(thicknessLabel);
    mainLayout->addWidget(thicknessEdit);
    mainLayout->addWidget(heightLabel);
    mainLayout->addWidget(heightEdit);
    mainLayout->addWidget(ageLabel);
    mainLayout->addWidget(ageEdit);
    setLayout(mainLayout);
}

CharCreationRelationsTab::CharCreationRelationsTab(QWidget* parent) : QWidget(parent){
    QLabel* franchiseReadLabel = new QLabel(tr("Franchise"));
    franchiseEdit = new QLineEdit("");
    
    QLabel* professionReadLabel = new QLabel(tr("Profession"));
    professionEdit = new QLineEdit("");
    
    QLabel* nationalityLabel = new QLabel(tr("Nationality"));
    nationalityEdit = new QLineEdit("American");

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(franchiseReadLabel);
    mainLayout->addWidget(franchiseEdit);
    mainLayout->addWidget(professionReadLabel);
    mainLayout->addWidget(professionEdit);
    mainLayout->addWidget(nationalityLabel);
    mainLayout->addWidget(nationalityEdit);
    setLayout(mainLayout);
}

CharCreationAttractionsTab::CharCreationAttractionsTab(QWidget* parent) : QWidget(parent){
    QLabel* attractToLabel1 = new QLabel(tr("Attractions"));
    QLabel* attractToLabel2 = new QLabel(tr("1\tSame"));
    QLabel* attractToLabel3 = new QLabel(tr("2\tOther"));
    
    QLabel* attractToGenderLabel = new QLabel(tr("Gender"));
    // asexual=0, homosexual=1, heterosexual=2, bisexual=3
    attractToGenderEdit = new QLineEdit("3");
    
    QLabel* attractToSpeciesLabel = new QLabel(tr("Species"));
    // asexual=0, normal=1, exclusive zoophile=2, zoophile=3
    attractToSpeciesEdit = new QLineEdit("1");
    
    QLabel* attractToRaceLabel = new QLabel(tr("Race"));
    attractToRaceEdit = new QLineEdit("3");
    

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(attractToLabel1);
    mainLayout->addWidget(attractToLabel2);
    mainLayout->addWidget(attractToLabel3);
    mainLayout->addWidget(attractToGenderLabel);
    mainLayout->addWidget(attractToGenderEdit);
    mainLayout->addWidget(attractToSpeciesLabel);
    mainLayout->addWidget(attractToSpeciesEdit);
    mainLayout->addWidget(attractToRaceLabel);
    mainLayout->addWidget(attractToRaceEdit);
    setLayout(mainLayout);
}

int stoi_or_null(const char* str){
    int n = 0;
    if (str[0] != 0)
        n = std::stoi(str);
    return n;
}

int stoi_or_null__attract(const char* str){
    int n = stoi_or_null(str);
    if (n == 0)
        return 0;
    return (n << 1) | 1;
}

Character CharCreationDialog::get_data() const{
    Character data = {
        invariantsTab->nameEdit->text().toLocal8Bit().data(),
        stoi_or_null(invariantsTab->sexIDEdit->text().toLocal8Bit().data()),
        invariantsTab->speciesEdit->text().toLocal8Bit().data(),
        invariantsTab->raceEdit->text().toLocal8Bit().data(),
        0, //invariantsTab->eyeColourEdit->text().toLocal8Bit().data(),
        
        0, //variantsTab->skinColourEdit->text().toLocal8Bit().data(),
        0, //variantsTab->hairColourEdit->text().toLocal8Bit().data(),
        stoi_or_null(variantsTab->thicknessEdit->text().toLocal8Bit().data()),
        stoi_or_null(variantsTab->heightEdit->text().toLocal8Bit().data()),
        stoi_or_null(variantsTab->ageEdit->text().toLocal8Bit().data()),
        
        relationsTab->franchiseEdit->text().toLocal8Bit().data(),
        relationsTab->professionEdit->text().toLocal8Bit().data(),
        relationsTab->nationalityEdit->text().toLocal8Bit().data(),
        
        stoi_or_null__attract(attractionsTab->attractToGenderEdit->text().toLocal8Bit().data()),
        stoi_or_null__attract(attractionsTab->attractToSpeciesEdit->text().toLocal8Bit().data()),
        stoi_or_null__attract(attractionsTab->attractToRaceEdit->text().toLocal8Bit().data())
    };
    return data;
}
