/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "instance_relation_unlink_tag_btn.hpp"

#include <compsky/mysql/query.hpp>


namespace _mysql {
	extern MYSQL* obj;
}

extern char BUF[];


InstanceRelationUnlinkTagBtn::InstanceRelationUnlinkTagBtn(const uint64_t _relation_id,  const uint64_t _tag_id,  QWidget* parent)
: QPushButton("Unlink", parent)
, relation_id(_relation_id)
, tag_id(_tag_id)
{}

void InstanceRelationUnlinkTagBtn::exec(){
	compsky::mysql::exec(_mysql::obj,  BUF,  "DELETE FROM relation2tag WHERE tag_id=", this->tag_id, " AND relation_id=", this->relation_id);
}

void InstanceRelationUnlinkTagBtn::mousePressEvent(QMouseEvent* e){
	switch(e->button()){
		case Qt::LeftButton:
			return this->exec();
		default: return;
	}
}
