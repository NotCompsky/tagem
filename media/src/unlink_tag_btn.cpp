/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "unlink_tag_btn.hpp"
#include "info_dialog.hpp"

#include <compsky/mysql/query.hpp>

#include <QMessageBox>


namespace _mysql {
	extern MYSQL* obj;
}

extern char BUF[4096];


UnlinkTagBtn::UnlinkTagBtn(const uint64_t id,  QWidget* parent) : QPushButton("Unlink", parent), tag_id(id) {}

void UnlinkTagBtn::exec(){
	compsky::mysql::exec(_mysql::obj,  BUF,  "DELETE FROM file2tag WHERE tag_id=", this->tag_id, " AND file_id=", static_cast<InfoDialog*>(this->parent())->file_id);
}

void UnlinkTagBtn::mousePressEvent(QMouseEvent* e){
	switch(e->button()){
		case Qt::LeftButton:
			return this->exec();
		default: return;
	}
}
