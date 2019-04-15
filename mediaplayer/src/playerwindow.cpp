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
    key_receiver->window = this;
    this->installEventFilter(key_receiver);
    
    for (auto i=0; i<10; ++i)
        tag_preset[i] = "";
    
    sql_driver = get_driver_instance();
    sql_con = sql_driver->connect("unix:///var/run/mysqld/mysqld.sock", USERNAME, PASSWORD);
    sql_con->setSchema("mytags");
    sql_stmt = sql_con->createStatement();
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

const char* sql_stmt__insert_into_fileid2tagid = "INSERT INTO files (fp) values(\"";
#define len_lkfdigdlofjg 31
const char* sql_stmt__select_from_fileid2tagid = "SELECT id FROM files WHERE fp = \"";
#define len_odfikjgdfigd 33


QString PlayerWindow::media_tag(QString str){
    if (media_fp == NULL)
        return "";
    
    // Triggered on key press
    bool ok;
    QString tagstr = QInputDialog::getText(this, tr("Get Tag"), tr("Tag"), QLineEdit::Normal, str, &ok);
    if (ok && !tagstr.isEmpty())
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
    memcpy(statement, sql_stmt__select_from_fileid2tagid, len_odfikjgdfigd);
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
        
        memcpy(statement, sql_stmt__insert_into_fileid2tagid, len_lkfdigdlofjg);
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



















bool keyReceiver::eventFilter(QObject* obj, QEvent* event)
// src: https://wiki.qt.io/How_to_catch_enter_key
{
    if (event->type()==QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        switch(key->key()){
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_D:
                window->media_open(); // Causes SEGFAULT, even though clicking on "Next" button is fine.
                break;
            case Qt::Key_T:
                window->media_tag("");
                window->media_open();
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
                window->media_tag(window->tag_preset[1]);   window->media_open();
                break;
            case Qt::Key_Exclam:
                window->media_tag_new_preset(1);            window->media_open();
                break;
            case Qt::Key_2:
                window->media_tag(window->tag_preset[2]);   window->media_open();
                break;
            case Qt::Key_QuoteDbl:
                window->media_tag_new_preset(2);            window->media_open();
                break;
            case Qt::Key_3:
                window->media_tag(window->tag_preset[3]);   window->media_open();
                break;
            case Qt::Key_sterling:
                window->media_tag_new_preset(3);            window->media_open();
                break;
            case Qt::Key_4:
                window->media_tag(window->tag_preset[4]);   window->media_open();
                break;
            case Qt::Key_Dollar:
                window->media_tag_new_preset(4);            window->media_open();
                break;
            case Qt::Key_5:
                window->media_tag(window->tag_preset[5]);   window->media_open();
                break;
            case Qt::Key_Percent:
                window->media_tag_new_preset(5);            window->media_open();
                break;
            case Qt::Key_6:
                window->media_tag(window->tag_preset[6]);   window->media_open();
                break;
            case Qt::Key_AsciiCircum:
                window->media_tag_new_preset(6);            window->media_open();
                break;
            case Qt::Key_7:
                window->media_tag(window->tag_preset[7]);   window->media_open();
                break;
            case Qt::Key_Ampersand:
                window->media_tag_new_preset(7);            window->media_open();
                break;
            case Qt::Key_8:
                window->media_tag(window->tag_preset[8]);   window->media_open();
                break;
            case Qt::Key_Asterisk:
                window->media_tag_new_preset(8);            window->media_open();
                break;
            case Qt::Key_9:
                window->media_tag(window->tag_preset[9]);   window->media_open();
                break;
            case Qt::Key_ParenLeft:
                window->media_tag_new_preset(9);            window->media_open();
                break;
            case Qt::Key_0:
                window->media_tag(window->tag_preset[0]);   window->media_open();
                break;
            case Qt::Key_ParenRight:
                window->media_tag_new_preset(0);            window->media_open();
                break;
            
            default: return QObject::eventFilter(obj, event);
        }
        return true;
    }
    return QObject::eventFilter(obj, event);
}
