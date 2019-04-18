/******************************************************************************
    Simple Player:  this file is part of QtAV examples
    Copyright (C) 2012-2016 Wang Bin <wbsecg1@gmail.com>

*   This file is part of QtAV

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "playerwindow.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QMessageBox>
#include <QDebug>
#include <QInputDialog>
#include <QKeyEvent>


#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>


// TODO: Add character status (injuries, mood, wearing_glasses, etc.)


// mysql.files.media_id is the ID of the unique image/scene, irrespective of rescaling, recolouring, etc.


using namespace QtAV;

PlayerWindow::PlayerWindow(QWidget *parent) : QWidget(parent)
{
    m_unit = 1000;
    setWindowTitle(QString::fromLatin1("QtAV simple player example"));
    m_player = new AVPlayer(this);
    QVBoxLayout *vl = new QVBoxLayout();
    setLayout(vl);
    m_vo = new VideoOutput(this);
    if (!m_vo->widget()) {
        QMessageBox::warning(0, QString::fromLatin1("QtAV error"), tr("Can not create video renderer"));
        return;
    }
    m_player->setRenderer(m_vo);
    vl->addWidget(m_vo->widget());
    m_slider = new QSlider();
    m_slider->setOrientation(Qt::Horizontal);
    connect(m_slider, SIGNAL(sliderMoved(int)), SLOT(seekBySlider(int)));
    connect(m_slider, SIGNAL(sliderPressed()), SLOT(seekBySlider()));
    connect(m_player, SIGNAL(positionChanged(qint64)), SLOT(updateSlider(qint64)));
    connect(m_player, SIGNAL(started()), SLOT(updateSlider()));
    connect(m_player, SIGNAL(notifyIntervalChanged()), SLOT(updateSliderUnit()));
    
    vl->addWidget(m_slider);
    
    inf = fopen("/tmp/mpv.sock", "r");
    
    media_fp = NULL;
    
    keyReceiver* key_receiver = new keyReceiver();
    key_receiver->state = key_receiver->state_default;
    key_receiver->window = this;
    this->installEventFilter(key_receiver);
    
    for (auto i=0; i<10; ++i)
        tag_preset[i] = "";
    
    sql_driver = get_driver_instance();
    sql_con = sql_driver->connect("unix:///var/run/mysqld/mysqld.sock", USERNAME, PASSWORD);
    sql_con->setSchema("mytags");
    sql_stmt = sql_con->createStatement();
    
    charcreation_dialog = new CharCreationDialog();
    charcreation_dialog->setModal(true);
}

void PlayerWindow::media_open()
{
    // WARNING: fp MUST be initialised, unless called via signal button press
    size_t fp_size;
    if (getline(&media_fp, &fp_size, inf) == -1)
        close();
    QString file = "";
    auto strlen_fp = strlen(media_fp);
    media_fp[strlen_fp-1] = 0; // Last char is \n
    for (auto i=0; i<strlen_fp-1; ++i)
        file += media_fp[i];
    
    if (file.isEmpty())
        return;
    
    //qDebug() << "media_open " << file; // SegFault without this line
    m_player->play(file);
}

void PlayerWindow::seekBySlider(int value)
{
    if (!m_player->isPlaying())
        return;
    m_player->seek(qint64(value*m_unit));
}

void PlayerWindow::seekBySlider()
{
    seekBySlider(m_slider->value());
}

void PlayerWindow::playPause()
{
    if (!m_player->isPlaying()) {
        m_player->play();
        return;
    }
    m_player->pause(!m_player->isPaused());
}

void PlayerWindow::updateSlider(qint64 value)
{
    m_slider->setRange(0, int(m_player->duration()/m_unit));
    m_slider->setValue(int(value/m_unit));
}

void PlayerWindow::updateSlider()
{
    updateSlider(m_player->position());
}

void PlayerWindow::updateSliderUnit()
{
    m_unit = m_player->notifyInterval();
    updateSlider();
}

const char* sql_stmt__insert_into_tags = "INSERT INTO tags (name) values(\"";
#define len_sdflkgdfgffg 32
const char* sql_stmt__select_from_tags = "SELECT id FROM tags WHERE name = \"";
#define len_oijerfjgfdgg 34

const char* sql_stmt__insert_into_files = "INSERT INTO files (fp) values(\"";
#define len_lkfdigdlofjg 31
const char* sql_stmt__select_from_files = "SELECT id FROM files WHERE fp = \"";
#define len_odfikjgdfigd 33

const char* sql_stmt__insert_into_tag2file = "INSERT INTO tag2file (file_id, tag_id) values(";
#define len_lkfdigdlofjh 46
const char* sql_stmt__select_from_tag2file = "SELECT id FROM tag2file WHERE (file_id, tag_id) = (";
#define len_odfikjgdfigh 51

const char* sql_stmt__insert_into_chars = "INSERT INTO chars (name, sex_id, species_id, race_id, skincolour_id, haircolour_id, eyecolour_id, age, wears_specs, franchise_id, profession_id) = (";
#define len_sqlstmtinsertintochars 148
const char* sql_stmt__select_from_chars = "SELECT id FROM chars WHERE name = \"";
#define len_sqlstmtselectfromchars 35

#define ASCII_OFFSET 48

QString PlayerWindow::media_tag(QString str){
    if (media_fp == NULL)
        return "";
    
    // Triggered on key press
    bool ok;
    QString tagstr = QInputDialog::getText(this, tr("Get Tag"), tr("Tag"), QLineEdit::Normal, str, &ok);
    if (!ok || tagstr.isEmpty())
        return "";
    
    qDebug() << "Tag: " << tagstr;
    
    QByteArray tagstr_ba = tagstr.toLocal8Bit();
    const char* tagchars = tagstr_ba.data();
    
    char statement[1024 + 42 + 2 + 1];
    int i;
    
    
    i = strlen(tagchars);
    
    
    goto__mdsfgdfgdf:
    memcpy(statement, sql_stmt__select_from_tags, len_oijerfjgfdgg);
    memcpy(statement + len_oijerfjgfdgg, tagchars, i);
    i += len_oijerfjgfdgg;
    statement[i++] = '\"';
    statement[i++] = ';';
    statement[i] = 0;
    i -= 2;
    i -= len_oijerfjgfdgg;
    qDebug() << statement;
    sql_res = sql_stmt->executeQuery(statement);
    
    int tag_id;
    
    if (sql_res->next())
        tag_id = sql_res->getInt(1); // 1 is first column
    else {
        qDebug() << "No prior tags of this value";
        
        memcpy(statement, sql_stmt__insert_into_tags, len_sdflkgdfgffg);
        memcpy(statement + len_sdflkgdfgffg, tagchars, i);
        i += len_sdflkgdfgffg;
        statement[i++] = '\"';
        statement[i++] = ')';
        statement[i++] = ';';
        statement[i] = 0;
        i -= 3;
        i -= len_sdflkgdfgffg;
        qDebug() << statement;
        sql_stmt->execute(statement);
        
        goto goto__mdsfgdfgdf;
    }
    
    //sql_res = sql_stmt->executeQuery("SELECT LAST_INSERT_ID() AS id;");
    
    qDebug() << "tag_id: " << tag_id;
    
    
    
    i = strlen(media_fp);
    
    goto__mdsfgdfgda:
    memcpy(statement, sql_stmt__select_from_files, len_odfikjgdfigd);
    memcpy(statement + len_odfikjgdfigd, media_fp, i);
    i += len_odfikjgdfigd;
    statement[i++] = '\"';
    statement[i++] = ';';
    statement[i] = 0;
    i -= 2;
    i -= len_odfikjgdfigd;
    qDebug() << statement;
    sql_res = sql_stmt->executeQuery(statement);
    
    int file_id;
    
    if (sql_res->next())
        file_id = sql_res->getInt(1); // 1 is first column
    else {
        qDebug() << "No prior tags of this value";
        
        memcpy(statement, sql_stmt__insert_into_files, len_lkfdigdlofjg);
        memcpy(statement + len_lkfdigdlofjg, media_fp, i);
        i += len_lkfdigdlofjg;
        statement[i++] = '\"';
        statement[i++] = ')';
        statement[i++] = ';';
        statement[i] = 0;
        i -= 3;
        i -= len_lkfdigdlofjg;
        qDebug() << statement;
        sql_stmt->execute(statement);
        
        goto goto__mdsfgdfgda;
    }
    
    //sql_res = sql_stmt->executeQuery("SELECT LAST_INSERT_ID() AS id;");
    
    qDebug() << "file_id: " << file_id;
    
    
    
    
    
    goto__mdsfgdfgdh:
    i = 0;
    memcpy(statement + i, sql_stmt__select_from_tag2file, len_odfikjgdfigh);
    i += len_odfikjgdfigh;
    for (int j=file_id;  j>0;  j/=10)
        statement[i++] = ASCII_OFFSET + (j % 10);
    statement[i++] = ',';
    statement[i++] = ' ';
    for (int j=tag_id;  j>0;  j/=10)
        statement[i++] = ASCII_OFFSET + (j % 10);
    statement[i++] = ')';
    statement[i++] = ';';
    statement[i] = 0;
    
    qDebug() << statement;
    sql_res = sql_stmt->executeQuery(statement);
    
    int tag2file_id;
    
    if (sql_res->next())
        tag2file_id = sql_res->getInt(1); // 1 is first column
    else {
        qDebug() << "No prior tags of this value";
        
        i = 0;
        memcpy(statement + i, sql_stmt__insert_into_tag2file, len_lkfdigdlofjh);
        i += len_lkfdigdlofjh;
        for (int j=file_id;  j>0;  j/=10)
            statement[i++] = ASCII_OFFSET + (j % 10);
        statement[i++] = ',';
        statement[i++] = ' ';
        for (int j=tag_id;  j>0;  j/=10)
            statement[i++] = ASCII_OFFSET + (j % 10);
        statement[i++] = ')';
        statement[i++] = ';';
        statement[i] = 0;
        qDebug() << statement;
        sql_stmt->execute(statement);
        
        goto goto__mdsfgdfgdh;
    }
    
    //sql_res = sql_stmt->executeQuery("SELECT LAST_INSERT_ID() AS id;");
    
    qDebug() << "tag2file_id: " << tag2file_id;
    
    
    
    return tagstr;
}

void PlayerWindow::media_tag_new_preset(int n){
    tag_preset[n] = media_tag(tag_preset[n]);
}

void PlayerWindow::media_overwrite(){
    // Triggered on key press
    bool ok;
    QString str = QInputDialog::getText(this, tr("Overwrite"), tr("Value"), QLineEdit::Normal, "Manually deleted", &ok);
    if (ok && !str.isEmpty())
        qDebug() << "Overwritten with: " << str;
}

void PlayerWindow::media_delete(){
    qDebug() << "Deleted: " << media_fp;
}

int PlayerWindow::search_for_char(const char* name){
    int i = 0;
    int char_id = 0;
    char statement[len_sqlstmtselectfromchars + 128 + 3];
    
    memcpy(statement + i, sql_stmt__select_from_chars, len_sqlstmtselectfromchars);
    i += len_sqlstmtselectfromchars;
    memcpy(statement + i, name, strlen(name));
    i += strlen(name);
    statement[i++] = '"';
    statement[i++] = ';';
    statement[i] = 0;
    
    qDebug() << statement;
    sql_res = sql_stmt->executeQuery(statement);
    
    if (sql_res->next())
        char_id = sql_res->getInt(1); // 1 is first column
    
    return char_id;
}

void PlayerWindow::add_character(){
    qDebug() << "add_character()";
    
    bool ok;
    const char* name = QInputDialog::getText(this, tr("Character Name"), tr("Name"), QLineEdit::Normal, "", &ok).toLocal8Bit().data();
    if (!ok)
        return;
    
    int i;
    int char_id = 0;
    char statement[len_sqlstmtselectfromchars + 1024];
    
    if (name[0] != 0)
        char_id = search_for_char(name);
    
    if (char_id == 0){
        goto__createchar:
        
        if (charcreation_dialog->exec() == QDialog::Accepted){
            Character data = charcreation_dialog->get_data();
            const char* name        = data.name;
            const int sex_id        = data.sex_id;
            const char* species     = data.species;
            const char* race        = data.race;
            const int eyecolour     = data.eyecolour;
            
            const int skincolour    = data.skincolour;
            const int haircolour    = data.haircolour;
            const int thickness     = data.thickness;
            const int height        = data.height;
            const int age           = data.age;
            
            const char* franchise   = data.franchise;
            const char* profession  = data.profession;
            const char* nationality = data.nationality;
            
            const int attract_to_gender     = data.attract_to_gender;
            const int attract_to_species    = data.attract_to_species;
            const int attract_to_race       = data.attract_to_race;
            
            
            const int species_id        = get_id_from_table("species", species);
            const int race_id           = get_id_from_table("races", race);
            const int franchise_id      = get_id_from_table("franchises", franchise);
            const int profession_id     = get_id_from_table("professions", profession);
            const int nationality_id    = get_id_from_table("nationalities", nationality);
            
            
            char statement[4096];
            
            
            sprintf("INSERT INTO chars (name, sex_id, species_id, race_id, eyecolour, franchise_id, thickness, height) = (\"%s\", %d, %d, %d, %d, %d)", statement, name, sex_id, species_id, race_id, eyecolour, franchise_id);
            
            qDebug() << statement;
            sql_stmt->execute(statement);
            
            if (name[0] != 0)
                char_id = search_for_char(name);
            
            
            sprintf("INSERT INTO char_instances (char_id, skincolour, haircolour, age, profession_id, nationality_id, thickness, height, attract_to_gender, attract_to_species, attract_to_race) = (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", statement, char_id, skincolour, haircolour, age, profession_id, nationality_id, thickness, height, attract_to_gender, attract_to_species, attract_to_race);
            
            qDebug() << statement;
            sql_stmt->execute(statement);
            
            /*
CREATE TABLE species (
    id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARCHAR(128),
    PRIMARY KEY (id)
);

CREATE TABLE races (
    id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    species_id INT UNSIGNED,
    name VARCHAR(128),
    PRIMARY KEY (id)
);

CREATE TABLE franchises (
    id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARCHAR(128),
    PRIMARY KEY (id)
);

CREATE TABLE professions (
    id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARCHAR(128),
    PRIMARY KEY (id)
);

CREATE TABLE nationalities (
    id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARCHAR(128),
    PRIMARY KEY (id)
);

CREATE TABLE chars (
    id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    
    name VARCHAR(128),
    sex_id INT UNSIGNED,
    species_id INT UNSIGNED,
    race_id INT UNSIGNED,
    eyecolour INT UNSIGNED,
    
    franchise_id INT UNSIGNED,
    
    created_on timestamp DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id)
);

CREATE TABLE char_instances (
    id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    
    char_id INT UNSIGNED,
    
    skincolour INT UNSIGNED,
    haircolour INT UNSIGNED,
    thickness INT UNSIGNED,
    height INT UNSIGNED,
    age INT UNSIGNED,
    
    profession_id INT UNSIGNED,
    nationality_id INT UNSIGNED,
    
    attract_to_gender INT UNSIGNED,
    attract_to_species INT UNSIGNED,
    attract_to_race INT UNSIGNED,
    
    created_on timestamp DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id)
);

GRANT CREATE ON mytags.species TO marx@localhost;
GRANT SELECT ON mytags.species TO marx@localhost;
GRANT CREATE ON mytags.races TO marx@localhost;
GRANT SELECT ON mytags.races TO marx@localhost;
GRANT CREATE ON mytags.franchises TO marx@localhost;
GRANT SELECT ON mytags.franchises TO marx@localhost;
GRANT CREATE ON mytags.professions TO marx@localhost;
GRANT SELECT ON mytags.professions TO marx@localhost;
GRANT CREATE ON mytags.nationalities TO marx@localhost;
GRANT SELECT ON mytags.nationalities TO marx@localhost;
GRANT CREATE ON mytags.chars TO marx@localhost;
GRANT SELECT ON mytags.chars TO marx@localhost;
GRANT CREATE ON mytags.char_instances TO marx@localhost;
GRANT SELECT ON mytags.char_instances TO marx@localhost;
            */
            
            return tag_as_char(char_id);
        } else {
            qDebug() << "Cancelled";
            return;
        }
    }
    return tag_as_char(char_id);
}

int PlayerWindow::get_id_from_table(const char* table_name, const char* entry_name){
    if (entry_name[0] == 0)
        return 0;
    
    int i;
    char statement[1024] = "SELECT id FROM ";
    
    goto__select_from_table:
    i = 0;
    i += strlen(statement);
    memcpy(statement + i,  table_name,  strlen(table_name));
    i += strlen(table_name);
    statement[i++] = ' ';
    statement[i++] = ',';
    statement[i++] = '"';
    memcpy(statement + i,  entry_name,  strlen(entry_name));
    i += strlen(entry_name);
    statement[i++] = '"';
    statement[i++] = ';';
    statement[i] = 0;
    
    sql_stmt->executeQuery(statement);
    
    if (sql_res->next())
        // Entry already existed in table
        return sql_res->getInt(1);
    
    i = 0;
    const char* statement2 = "INSERT INTO ";
    memcpy(statement, statement2, strlen(statement2));
    i += strlen(statement);
    memcpy(statement + i,  table_name,  strlen(table_name));
    i += strlen(table_name);
    const char* fff = " (name) values(";
    memcpy(statement + i,  fff,  strlen(fff));
    i += strlen(fff);
    memcpy(statement + i,  entry_name,  strlen(entry_name));
    i += strlen(entry_name);
    statement[i++] = ')';
    statement[i++] = ';';
    sql_stmt->execute(statement);
    goto goto__select_from_table;
}

void PlayerWindow::tag_as_char(int char_id){
    qDebug() << "tag_as_char(" << +char_id << ")";
}








std::map<const int, const int> key2n = {
    {Qt::Key_1, 1},
    {Qt::Key_2, 2},
    {Qt::Key_3, 3},
    {Qt::Key_4, 4},
    {Qt::Key_5, 5},
    {Qt::Key_6, 6},
    {Qt::Key_7, 7},
    {Qt::Key_8, 8},
    {Qt::Key_9, 9},
    {Qt::Key_0, 0},
    {Qt::Key_Exclam, 1},
    {Qt::Key_QuoteDbl, 2},
    {Qt::Key_sterling, 3},
    {Qt::Key_Dollar, 4},
    {Qt::Key_Percent, 5},
    {Qt::Key_AsciiCircum, 6},
    {Qt::Key_Ampersand, 7},
    {Qt::Key_Asterisk, 8},
    {Qt::Key_ParenLeft, 9},
    {Qt::Key_ParenRight, 0},
};




bool keyReceiver::eventFilter(QObject* obj, QEvent* event)
// src: https://wiki.qt.io/How_to_catch_enter_key
{
    if (event->type()==QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        switch(int keyval = key->key()){
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_C:
                if (state == state_clicked){
                    window->add_character();
                    state = state_default;
                }
                break;
            case Qt::Key_D:
                window->media_open(); // Causes SEGFAULT, even though clicking on "Next" button is fine.
                break;
            case Qt::Key_T:
                window->media_tag("");
                break;
            case Qt::Key_O:
                window->media_overwrite();
                break;
            case Qt::Key_Q:
                window->close();
                break;
            case Qt::Key_X:
                window->media_delete();
                window->media_open();
                break;
            case Qt::Key_Space:
                window->playPause();
                break;
            
            /* Preset Tags */
            // N to open tag dialog and paste Nth preset into tag field, SHIFT+N to open tag dialog and set user input as Nth preset
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
            case Qt::Key_0:
                window->media_tag(window->tag_preset[key2n[keyval]]);
                break;
            case Qt::Key_Exclam:
            case Qt::Key_QuoteDbl:
            case Qt::Key_sterling:
            case Qt::Key_Dollar:
            case Qt::Key_Percent:
            case Qt::Key_AsciiCircum:
            case Qt::Key_Ampersand:
            case Qt::Key_Asterisk:
            case Qt::Key_ParenLeft:
            case Qt::Key_ParenRight:
                window->media_tag_new_preset(key2n[keyval]);
                break;
            
            default: return QObject::eventFilter(obj, event);
        }
        return true;
    } else if (event->type()==QEvent::MouseButtonRelease){
        state = state_clicked;
    }
    return QObject::eventFilter(obj, event);
}
