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
#include "utils.h" // for count_digits, itoa_nonstandard
#include <cstdio> // for remove
#include <unistd.h> // for symlink
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
    connect(m_player, SIGNAL(started()), SLOT(set_player_options_for_img()));
    
    vl->addWidget(m_slider);
    
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
    
    this->volume = 0.1;
    this->m_player->audio()->setVolume(this->volume);
}

void PlayerWindow::set_player_options_for_img(){
    qDebug() << "Duration " << +this->m_player->duration();
    if (this->m_player->duration() == 40){
        qDebug() << "Auto paused";
        this->m_player->pause(true);
    }
}

void PlayerWindow::media_open()
{
    // WARNING: fp MUST be initialised, unless called via signal button press
    size_t fp_size;
    if (getline(&media_fp, &fp_size, stdin) == -1)
        close();
    QString file = "";
    auto strlen_fp = strlen(media_fp);
    media_fp[strlen_fp-1] = 0; // Last char is \n
    for (auto i=0; i<strlen_fp-1; ++i)
        file += media_fp[i];
    
    if (file.isEmpty())
        return;
    
    this->file_id_len = 0; // Tells us that file_id hasn't been cached yet
    
    qDebug() << "media_open " << file; // SegFault without this line
    m_player->play(file);
    qDebug() << "m_player->duration(): " << +m_player->duration();
    m_player->setRepeat(-1); // Repeat infinitely
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

#define ASCII_OFFSET 48

int PlayerWindow::file_attr_id(const char* attr, int attr_id_int, const char* file_id, const int file_id_len){
    char stmt[1024];
    int i;
    
    int attr_id_len = count_digits(attr_id_int);
    char attr_id[attr_id_len + 1];
    itoa_nonstandard(attr_id_int, attr_id_len, attr_id);
    
    
    goto__fileattridselect:
    i = 0;
    
    const char* a = "SELECT id FROM ";
    memcpy(stmt + i,  a,  strlen(a));
    i += strlen(a);
    
    memcpy(stmt + i,  attr,  strlen(attr));
    i += strlen(attr);
    
    const char* b = "2file WHERE (file_id, ";
    memcpy(stmt + i,  b,  strlen(b));
    i += strlen(b);
    
    memcpy(stmt + i,  attr,  strlen(attr));
    i += strlen(attr);
    
    const char* c = "_id) = (\"";
    memcpy(stmt + i,  c,  strlen(c));
    i += strlen(c);
    
    memcpy(stmt + i,  file_id,  file_id_len);
    i += file_id_len;
    
    const char* d = "\", \"";
    memcpy(stmt + i,  d,  strlen(d));
    i += strlen(d);
    
    memcpy(stmt + i,  attr_id,  strlen(attr_id));
    i += strlen(attr_id);
    
    const char* e = "\");";
    memcpy(stmt + i,  e,  strlen(e));
    i += strlen(e);
    
    stmt[i] = 0;
    
    qDebug() << stmt;
    sql_res = sql_stmt->executeQuery(stmt);
    
    if (sql_res->next())
        return sql_res->getInt(1); // 1 is first column
    else
        qDebug() << "No prior tags of this value";
    
    
    i = 0;
    
    const char* f = "INSERT INTO ";
    memcpy(stmt + i,  f,  strlen(f));
    i += strlen(f);
    
    memcpy(stmt + i,  attr,  strlen(attr));
    i += strlen(attr);
    
    const char* g = "2file (file_id, ";
    memcpy(stmt + i,  g,  strlen(g));
    i += strlen(g);
    
    memcpy(stmt + i,  attr,  strlen(attr));
    i += strlen(attr);
    
    const char* h = "_id) values(\"";
    memcpy(stmt + i,  h,  strlen(h));
    i += strlen(h);
    
    memcpy(stmt + i,  file_id,  file_id_len);
    i += file_id_len;
    
    // ", "
    memcpy(stmt + i,  d,  strlen(d));
    i += strlen(d);
    
    memcpy(stmt + i,  attr_id,  strlen(attr_id));
    i += strlen(attr_id);
    
    memcpy(stmt + i,  e,  strlen(e));
    i += strlen(e);
    
    stmt[i] = 0;
    
    qDebug() << stmt;
    sql_stmt->execute(stmt);
    
    goto goto__fileattridselect; // Return the table entry id
}

QString PlayerWindow::media_tag(QString str){
    if (media_fp == NULL)
        return "";
    
    // Triggered on key press
    bool ok;
    QString tagstr = QInputDialog::getText(this, tr("Get Tag"), tr("Tag"), QLineEdit::Normal, str, &ok);
    if (!ok || tagstr.isEmpty())
        return "";
    
    QByteArray tagstr_ba = tagstr.toLocal8Bit();
    const char* tagchars = tagstr_ba.data();
    
    if (this->file_id_len == 0){
        int n = this->get_id_from_table("file", media_fp);
        this->file_id_len = count_digits(n);
        itoa_nonstandard(n, this->file_id_len, this->file_id);
        qDebug() << "Set file_id[" << +this->file_id_len << "]: " << this->file_id;
    }
    
    this->file_attr_id("tag",  this->get_id_from_table("tag", tagchars),  this->file_id,  this->file_id_len);
    
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
    qDebug() << "Deleting: " << this->media_fp;
    if (remove(this->media_fp) != 0)
        qDebug() << "Failed to delete: " << this->media_fp;
}

int PlayerWindow::search_for_char(const char* name){
    int i = 0;
    int char_id = 0;
    char statement[1024];
    
    const char* a = "SELECT id FROM person WHERE name = \"";
    memcpy(statement + i, a, strlen(a));
    i += strlen(a);
    
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
    char statement[2048];
    
    if (name[0] != 0)
        char_id = search_for_char(name);
    
    if (char_id == 0){
        goto__createchar:
        
        if (charcreation_dialog->exec() == QDialog::Accepted){
            Character data = charcreation_dialog->get_data();
            const char* name        = data.name;
            //sex_id
            const int species_id    = get_id_from_table("species", data.species);
            const int race_id       = get_id_from_table("race", data.race);
            //const int eyecolour
            
            //const int skincolour
            //const int haircolour
            //const int thickness
            //const int height
            //const int age
            
            const int franchise_id      = get_id_from_table("franchise", data.franchise);
            const int profession_id     = get_id_from_table("profession", data.profession);
            const int nationality_id    = get_id_from_table("nationality", data.nationality);
            
            //const int attract_to_gender
            //const int attract_to_species
            //const int attract_to_race
            
            
            char statement[4096];
            
            
            sprintf(statement, "INSERT INTO person (name, sex_id, species_id, race_id, eyecolour, franchise_id) values(\"%s\", %u, %u, %u, %u, %u);", name, data.sex_id, species_id, race_id, data.eyecolour, franchise_id);
            
            qDebug() << statement;
            sql_stmt->execute(statement);
            
            if (name[0] != 0)
                char_id = search_for_char(name);
            
            
            sprintf(statement, "INSERT INTO person_instance (char_id, skincolour, haircolour, age, profession_id, nationality_id, thickness, height, attract_to_gender, attract_to_species, attract_to_race) values(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u);", char_id, data.skincolour, data.haircolour, data.age, profession_id, nationality_id, data.thickness, data.height, data.attract_to_gender, data.attract_to_species, data.attract_to_race);
            
            qDebug() << statement;
            sql_stmt->execute(statement);
            
            free(data.name);
            free(data.species);
            free(data.race);
            free(data.franchise);
            free(data.profession);
            free(data.nationality);
            
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

CREATE TABLE person (
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

CREATE TABLE person_instance (
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

GRANT INSERT ON mytags.species TO marx@localhost;
GRANT SELECT ON mytags.species TO marx@localhost;
GRANT INSERT ON mytags.race TO marx@localhost;
GRANT SELECT ON mytags.race TO marx@localhost;
GRANT INSERT ON mytags.franchise TO marx@localhost;
GRANT SELECT ON mytags.franchise TO marx@localhost;
GRANT INSERT ON mytags.profession TO marx@localhost;
GRANT SELECT ON mytags.profession TO marx@localhost;
GRANT INSERT ON mytags.nationality TO marx@localhost;
GRANT SELECT ON mytags.nationality TO marx@localhost;
GRANT INSERT ON mytags.person TO marx@localhost;
GRANT SELECT ON mytags.person TO marx@localhost;
GRANT INSERT ON mytags.person_instance TO marx@localhost;
GRANT SELECT ON mytags.person_instance TO marx@localhost;
            */
            /*
NOTE
We require 0 to be an allowed value for all optional secondary fields
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
    char statement[2048];
    
    goto__select_from_table:
    
    i = 0;
    
    const char* a = "SELECT id FROM ";
    memcpy(statement + i,  a,  strlen(a));
    i += strlen(a);
    
    memcpy(statement + i,  table_name,  strlen(table_name));
    i += strlen(table_name);
    
    const char* dummy = " WHERE name = \"";
    memcpy(statement + i,  dummy,  strlen(dummy));
    i += strlen(dummy);
    
    memcpy(statement + i,  entry_name,  strlen(entry_name));
    i += strlen(entry_name);
    
    statement[i++] = '"';
    statement[i++] = ';';
    statement[i] = 0;
    
    qDebug() << statement;
    sql_res = sql_stmt->executeQuery(statement);
    
    if (sql_res->next()){
        // Entry already existed in table
        qDebug() << "SQL ID: " << +sql_res->getInt(1);
        return sql_res->getInt(1);
    }
    
    qDebug() << "Creating new entry in " << table_name << " for: " << entry_name;
    
    i = 0;
    const char* statement2 = "INSERT INTO ";
    memcpy(statement + i,  statement2,  strlen(statement2));
    i += strlen(statement2);
    
    memcpy(statement + i,  table_name,  strlen(table_name));
    i += strlen(table_name);
    
    const char* fff = " (name) values(\"";
    memcpy(statement + i,  fff,  strlen(fff));
    i += strlen(fff);
    
    memcpy(statement + i,  entry_name,  strlen(entry_name));
    i += strlen(entry_name);
    
    statement[i++] = '"';
    statement[i++] = ')';
    statement[i++] = ';';
    statement[i] = 0;
    
    qDebug() << statement;
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
            case Qt::Key_BracketLeft:
                if (window->volume > 0){
                    window->volume -= 0.05;
                    window->m_player->audio()->setVolume(window->volume);
                }
                break;
            case Qt::Key_BracketRight:
                if (window->volume < 1.25){
                    window->volume += 0.05;
                    window->m_player->audio()->setVolume(window->volume);
                }
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
