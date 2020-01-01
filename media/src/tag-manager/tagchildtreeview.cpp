#include "tagchildtreeview.hpp"

#include <compsky/mysql/mysql.h> // for MYSQL_RES*
#include <compsky/mysql/query.hpp>


namespace _mysql {
	extern MYSQL* obj;
}
extern MYSQL_RES* RES1;


TagChildTreeView::TagChildTreeView(QWidget* parent) : TagTreeView(true, parent) {
    this->setSelectionBehavior(this->SelectRows);
    this->setSelectionMode(this->SingleSelection);
    this->setDragDropMode(this->InternalMove);
    this->setDragDropOverwriteMode(false);
    
	compsky::mysql::query_buffer(
		_mysql::obj,
		RES1,
		"SELECT t2p.parent_id, t.id, IFNULL(A.c,0), t.name "
		"FROM tag2parent t2p "
		"JOIN tag t ON t.id=t2p.tag_id "
		"LEFT JOIN ("
			"SELECT tag_id, COUNT(file_id) AS `c` "
			"FROM file2tag "
			"GROUP BY tag_id"
		") A ON A.tag_id=t.id"
	);
    this->place_tags(0);
};
