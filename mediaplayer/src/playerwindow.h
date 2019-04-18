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

#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QWidget>
#include <QtAV>
#include <QTextEdit>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "create_char_wizard.h"


QT_BEGIN_NAMESPACE
class QSlider;
class QPushButton;
QT_END_NAMESPACE
class PlayerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerWindow(QWidget *parent = 0);
    QString media_tag(QString str);
    void media_tag_new_preset(int n);
    void media_overwrite();
    void media_open();
    void media_delete();
    void create_character(char** statement, const char* name);
    void add_character();
    void tag_as_char(int char_id);
    int get_id_from_table(const char* table_name, const char* entry_name);
    int search_for_char(const char*);
    QString tag_preset[10];
public Q_SLOTS:
    void seekBySlider(int value);
    void seekBySlider();
    void playPause();
private Q_SLOTS:
    void updateSlider(qint64 value);
    void updateSlider();
    void updateSliderUnit();

private:
    QtAV::VideoOutput *m_vo;
    QtAV::AVPlayer *m_player;
    QSlider *m_slider;
    int m_unit;
    char* media_fp;
    FILE* inf;
    
    sql::Driver* sql_driver;
    sql::Connection* sql_con;
    sql::Statement* sql_stmt;
    sql::ResultSet* sql_res;
    
    CharCreationDialog* charcreation_dialog;
};

class keyReceiver : public QObject
// src: https://wiki.qt.io/How_to_catch_enter_key
{
    Q_OBJECT
public:
    PlayerWindow* window;
    int state;
    enum { state_default, state_clicked };
protected:
    bool eventFilter(QObject* obj, QEvent* event);
};

#endif // PLAYERWINDOW_H
