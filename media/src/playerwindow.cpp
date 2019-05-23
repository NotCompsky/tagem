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
#include <QTimer>

#include "sql_utils.hpp" // for mysu::*, SQL_*

#define STDIN_FILENO 0

// mysql.files.media_id is the ID of the unique image/scene, irrespective of rescaling, recolouring, etc.


char STMT[4096];
char NOTE[30000];


using namespace QtAV;

TagDialog::TagDialog(QString title,  QString str,  QWidget *parent) : QDialog(parent){
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(buttonBox);
    nameEdit = new QLineEdit(str);
    mainLayout->addWidget(nameEdit);
    QLabel* guide = new QLabel(tr("Enter blank tag to designate as root tag"));
    mainLayout->addWidget(guide);
    this->setLayout(mainLayout);
    this->setWindowTitle(title);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QTimer::singleShot(0, nameEdit, SLOT(setFocus())); // Set focus after TagDialog instance is visible
}

PlayerWindow::PlayerWindow(int argc,  char** argv,  QWidget *parent) : QWidget(parent)
{
    this->ignore_tagged = false;
    for (auto i = 2;  i < argc;  ++i){
        const char* arg = argv[i];
        if (arg[1] == 0){
            switch(arg[0]){
                case 't': this->ignore_tagged = true; break;
                default: exit(2);
            }
        } else exit(3);
    }
    
    mysu::init(argv[1], "mytag");
    
    this->tagslist;
    SQL_RES = SQL_STMT->executeQuery("SELECT name FROM tag");
    while (SQL_RES->next()){
        this->tagslist << QString::fromStdString(SQL_RES->getString(1));
    }
    this->tagcompleter = new QCompleter(this->tagslist);
    
    m_unit = 1000;
    setWindowTitle(QString::fromLatin1("Media Tagger"));
    m_player = new AVPlayer(this);
    QVBoxLayout *vl = new QVBoxLayout();
    setLayout(vl);
    m_vo = new VideoOutput(this);
    if (!m_vo->widget()) {
        fprintf(stderr, "Cannot create QtAV renderer\n");
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
    
    media_fp[0] = 0;
    
    keyReceiver* key_receiver = new keyReceiver();
    key_receiver->state = key_receiver->state_default;
    key_receiver->window = this;
    this->installEventFilter(key_receiver);
    
    for (auto i=0; i<10; ++i)
        tag_preset[i] = "";
    
    this->volume = 0.1;
    this->m_player->audio()->setVolume(this->volume);
}

void PlayerWindow::set_player_options_for_img(){
    PRINTF("Duration: %d\n", this->m_player->duration());
    if (this->m_player->duration() == 40){
        PRINTF("Auto paused\n");
        this->m_player->pause(true);
    }
}

void PlayerWindow::media_next(){
    // TODO: Do not have different strings, but one fp string and lengths of dir and fname
    size_t size;
    char* dummy = nullptr;
    getline(&dummy, &size, stdin);
    memcpy(this->media_fp,  dummy,  strlen(dummy)-1); // Remove trailing newline
    this->media_fp[strlen(dummy)-1] = 0;
    
    this->media_dir_len = 0;
    for (auto i = 0;  this->media_fp[i] != 0;  ++i)
        if (this->media_fp[i] == '/')
            this->media_dir_len = i;
    
    memcpy(this->media_dir,  this->media_fp,  this->media_dir_len);
    this->media_dir[this->media_dir_len] = 0;
    
    auto media_fname_len = strlen(dummy)-1 - this->media_dir_len - 1;
    memcpy(this->media_fname,  this->media_fp + this->media_dir_len + 1,  media_fname_len);
    this->media_fname[media_fname_len] = 0;
    
    this->file_id_str_len = 0; // Tells us that file_id hasn't been cached yet
    
    this->media_open();
}

void PlayerWindow::media_open()
{
    if (this->ignore_tagged){
        this->ensure_fileID_set();
        uint64_t id;
        id = sql__get_first_file2attr_id(SQL_STMT, SQL_RES, "tag", this->file_id_str, this->file_id_str_len, id);
        if (id != 0){
            PRINTF("Skipped previously tagged: %s\n", this->media_fp);
            return this->media_next();
        }
    }
    
    // WARNING: fp MUST be initialised, unless called via signal button press
    QString file = this->media_fp;
    
    /* Set window title */
    QString fname = this->media_fname;
    this->setWindowTitle(fname);
    
    m_player->play(file);
    PRINTF("Duration: %d\n", this->m_player->duration());
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

void PlayerWindow::set_table_attr_by_id(const char* tbl, const char* id, const int id_len, const char* col, const char* val){
    int i;
    
    i = 0;
    
    constexpr const char* a = "UPDATE ";
    memcpy(STMT + i,  a,  strlen(a));
    i += strlen(a);
    
    memcpy(STMT + i,  tbl,  strlen(tbl));
    i += strlen(tbl);
    
    constexpr const char* b = " SET ";
    memcpy(STMT + i,  b,  strlen(b));
    i += strlen(b);
    
    memcpy(STMT + i,  col,  strlen(col));
    i += strlen(col);
    
    constexpr const char* c = " = ";
    memcpy(STMT + i,  c,  strlen(c));
    i += strlen(c);
    
    memcpy(STMT + i,  val,  strlen(val));
    i += strlen(val);
    
    constexpr const char* d = " WHERE id = ";
    memcpy(STMT + i,  d,  strlen(d));
    i += strlen(d);
    
    memcpy(STMT + i,  id,  id_len);
    i += id_len;
    
    STMT[i++] = ';';
    STMT[i] = 0;
    
    
    PRINTF("%s\n", STMT);
    SQL_STMT->execute(STMT);
}

uint64_t PlayerWindow::file_attr_id(const char* attr, uint64_t attr_id_int, const char* file_id, const int file_id_len){
    return sql__file_attr_id(SQL_STMT, SQL_RES, attr, attr_id_int, file_id, file_id_len);
}

void PlayerWindow::ensure_fileID_set(){
    if (this->file_id_str_len == 0){
        this->file_id = this->get_id_from_table("file", media_fp);
        this->file_id_str_len = count_digits(this->file_id);
        itoa_nonstandard(this->file_id, this->file_id_str_len, this->file_id_str);
        this->file_id_str[this->file_id_str_len] = 0;
    }
}

void PlayerWindow::media_score(){
    /*
    Rating of the unique media object itself. Not aspects unique to the file (such as resolution, or compresssion quality).
    
    It is applied to the 'file' table because there is no sense in having multiple files for the same unique media object. These perceptual duplicates will be listed themselves elsewhere.
    */
    bool ok;
    int score_int = QInputDialog::getInt(this, tr("Score"), tr("Score"), 0, -100, 100, 1, &ok);
    if (!ok)
        return;
    
    int n = count_digits(score_int);
    char score[n+1];
    itoa_nonstandard(score_int, n, score);
    score[n] = 0;
    
    this->ensure_fileID_set();
    
    this->set_table_attr_by_id("file", this->file_id_str, this->file_id_str_len, "score", score);
}

void PlayerWindow::media_note(){
    this->ensure_fileID_set();
    
    constexpr const char* a = "SELECT note FROM file WHERE id=";
    int i = 0;
    memcpy(STMT,  a,  strlen(a));
    i += strlen(a);
    memcpy(STMT + i,  this->file_id_str,  this->file_id_str_len);
    i += this->file_id_str_len;
    STMT[i] = 0;
    
    PRINTF("%s\n", STMT);
    SQL_RES = SQL_STMT->executeQuery(STMT);
    
    if (SQL_RES->next()){
        std::string s = SQL_RES->getString(1);
        memcpy(NOTE,  s.c_str(),  strlen(s.c_str()));
        NOTE[strlen(s.c_str())] = 0;
    } else NOTE[0] = 0;
    
    
    bool ok;
    QString str = QInputDialog::getMultiLineText(this, tr("Note"), tr("Note"), NOTE, &ok);
    if (!ok || str.isEmpty())
        return;
    QByteArray  bstr = str.toLocal8Bit();
    const char* cstr = bstr.data();
    auto j = 0;
    for (auto i = 0;  i < strlen(cstr);  ++i){
        if (cstr[i] == '"'  ||  cstr[i] == '\\')
            NOTE[j++] = '\\';
        NOTE[j++] = cstr[i];
    }
    NOTE[j] = 0;
    sql__update(SQL_STMT, SQL_RES, "file", "note", NOTE, this->file_id);
}

void PlayerWindow::tag2parent(uint64_t tagid,  uint64_t parid){
    constexpr const char* a = "INSERT IGNORE INTO tag2parent (tag_id, parent_id) VALUES (";
    int i = 0;
    memcpy(STMT,  a,  strlen(a));
    i += strlen(a);
    i += itoa_nonstandard(tagid,  STMT + i);
    STMT[i++] = ',';
    i += itoa_nonstandard(parid,  STMT + i);
    STMT[i++] = ')';
    STMT[i] = 0;
    
    printf("%s\n", STMT);
    SQL_STMT->execute(STMT);
}

uint64_t PlayerWindow::add_new_tag(QString tagstr,  uint64_t tagid){
    QByteArray tagstr_ba = tagstr.toLocal8Bit();
    const char* tagchars = tagstr_ba.data();
    
    this->tagslist.append(tagstr);
    delete this->tagcompleter;
    this->tagcompleter = new QCompleter(this->tagslist);
    
    if (tagid == 0)
        tagid = this->get_id_from_table("tag", tagchars);
    
    
    /* Get parent tag */
    TagDialog* tagdialog = new TagDialog("Parent Tag of", tagstr);
    tagdialog->nameEdit->setCompleter(this->tagcompleter);
    goto__cannotcancelparenttag:
    if (tagdialog->exec() != QDialog::Accepted)
        goto goto__cannotcancelparenttag;
    
    QString parent_tagstr = tagdialog->nameEdit->text();
    
    if (parent_tagstr.isEmpty()){
        this->tag2parent(tagid, 0);
        return tagid;
    }
    
    QByteArray parent_tagstr_ba = parent_tagstr.toLocal8Bit();
    char* parent_tagchars = parent_tagstr_ba.data();
    
    /* Insert parent-child relations */
    int lastindx = 0;
    for (auto i = 0;  ;  ++i)
        if (parent_tagchars[i] == '|'  ||  parent_tagchars[i] == 0){
            char iszero = parent_tagchars[i];
            parent_tagchars[i] = 0;
            
            printf("parent_tagchars: %s\n",  parent_tagchars + lastindx);
            auto parid = this->get_id_from_table("tag",  parent_tagchars + lastindx);
            
            this->tag2parent(tagid, parid);
            
            QString parstr = QString::fromLocal8Bit(parent_tagchars + lastindx);
            if (!this->tagslist.contains(parstr))
                this->add_new_tag(parstr, parid);
            
            if (iszero == 0)
                break;
            
            lastindx = i + 1;
        }
    
    return tagid;
}

QString PlayerWindow::media_tag(QString str){
    if (media_fp[0] == 0)
        return "";
    
    // Triggered on key press
    bool ok;
    TagDialog* tagdialog = new TagDialog("Tag", str);
    tagdialog->nameEdit->setCompleter(this->tagcompleter);
    if (tagdialog->exec() != QDialog::Accepted)
        return "";
    
    QString tagstr = tagdialog->nameEdit->text();
    
    if (tagstr.isEmpty())
        return "";
    
    uint64_t tagid;
    QByteArray tagstr_ba = tagstr.toLocal8Bit();
    const char* tagchars = tagstr_ba.data();
    if (!this->tagslist.contains(tagstr)){
        tagid = this->add_new_tag(tagstr);
    } else tagid = this->get_id_from_table("tag", tagchars);
    
    this->ensure_fileID_set();
    
    this->file_attr_id("tag",  tagid,  this->file_id_str,  this->file_id_str_len);
    
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
        PRINTF("Overwritten with: %s\n", str);
}

void PlayerWindow::media_replace_w_link(const char* src){
    PRINTF("Deleting: %s\n", this->media_fp);
    if (remove(this->media_fp) != 0)
        fprintf(stderr, "Failed to delete: %s\n", this->media_fp);
    if (symlink(src, this->media_fp) != 0)
        fprintf(stderr, "Failed to create ln2del symlink: %s\n", this->media_fp);
}

void PlayerWindow::media_delete(){
    this->media_replace_w_link("/home/compsky/bin/ln2del_ln");
}

void PlayerWindow::media_linkfrom(){
    bool ok;
    QString str = QInputDialog::getText(this, tr("Overwrite with link"), tr("Source path"), QLineEdit::Normal, "", &ok);
    if (!ok || str.isEmpty()){
        PRINTF("Cancelling media_linkfrom\n");
        return;
    }
    
    const char* src = str.toLocal8Bit().data();
    this->media_replace_w_link(src);
}

uint64_t PlayerWindow::get_id_from_table(const char* table_name, const char* entry_name){
    uint64_t value;
    return sql__get_id_from_table(SQL_STMT, SQL_RES, table_name, entry_name, value);
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
            case Qt::Key_D:
                window->media_next(); // Causes SEGFAULT, even though clicking on "Next" button is fine.
                break;
            case Qt::Key_L:
                window->media_linkfrom();
                break;
            case Qt::Key_S:
                window->media_score();
                break;
            case Qt::Key_N:
                window->media_note();
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
                window->media_next();
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
