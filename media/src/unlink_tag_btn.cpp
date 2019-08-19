/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "unlink_tag_btn.hpp"
#include "info_dialog.hpp"

#include <compsky/mysql/query.hpp>


namespace _mysql {
	extern MYSQL* obj;
}

extern char BUF[];


UnlinkTagBtn::UnlinkTagBtn(const char* const _delete_from_where,  const uint64_t _primary_id,  const char* const _and_tag_id_eql,  const uint64_t _tag_id,  QWidget* parent)
: QPushButton("Unlink", parent)
, delete_from_where(_delete_from_where)
, primary_id(_primary_id)
, and_tag_id_eql(_and_tag_id_eql)
, tag_id(_tag_id)
{}

void UnlinkTagBtn::exec(){
	compsky::mysql::exec(_mysql::obj,  BUF,  this->delete_from_where, this->primary_id, this->and_tag_id_eql, this->tag_id);
}

void UnlinkTagBtn::mousePressEvent(QMouseEvent* e){
	switch(e->button()){
		case Qt::LeftButton:
			return this->exec();
		default: return;
	}
}
