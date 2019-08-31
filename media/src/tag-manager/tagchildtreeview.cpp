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
    
    compsky::mysql::query_buffer(_mysql::obj, RES1,  "SELECT parent_id, B.id, B.c, B.name FROM tag2parent t2p LEFT JOIN (SELECT id, name, COUNT(A.tag_id) as c FROM tag LEFT JOIN(SELECT tag_id FROM file2tag) A ON A.tag_id = id GROUP BY id, name) B ON B.id = t2p.tag_id ORDER BY t2p.parent_id ASC");
    this->place_tags(0);
};
