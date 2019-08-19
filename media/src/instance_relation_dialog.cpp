#include "instance_relation_dialog.hpp"
#include "relation_hub.hpp"
#include "unlink_tag_btn.hpp"
#include "name_dialog.hpp"

#include <compsky/mysql/query.hpp>

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>


namespace _mysql {
	extern MYSQL* obj;
}

namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
}

extern MYSQL_RES* RES2;
extern MYSQL_ROW ROW2;
extern char BUF[];


InstanceRelationDialog::InstanceRelationDialog(const uint64_t _id,  MainWindow* const _win,  QWidget* parent)
: QDialog(parent)
, id(_id)
, win(_win)
{
	QVBoxLayout* l = new QVBoxLayout(this);
	
	l->addWidget(new QLabel("Tags"));
	
	compsky::mysql::query(_mysql::obj,  RES2,  BUF,  "SELECT t.id, t.name FROM tag t, relation2tag r2t WHERE t.id=r2t.tag_id AND r2t.relation_id=", _id);
	uint64_t _tag_id;
	char* _tag_name;
	while(compsky::mysql::assign_next_row(RES2, &ROW2, &_tag_id, &_tag_name)){
		QHBoxLayout* hbox = new QHBoxLayout;
		
		hbox->addWidget(new QLabel(_tag_name));
		hbox->addWidget(new UnlinkTagBtn("DELETE FROM relation2tag WHERE relation_id=", _id, " AND tag_id=", _tag_id, this));
		
		l->addLayout(hbox);
	}
	
	{
		QPushButton* btn = new QPushButton("+Tag", this);
		connect(btn, &QPushButton::clicked, this, &InstanceRelationDialog::add_tag);
		l->addWidget(btn);
	}
	
	{
		QPushButton* btn = new QPushButton("Hub", this);
		connect(btn, &QPushButton::clicked, this, &InstanceRelationDialog::display_hub);
		l->addWidget(btn);
	}
}

void InstanceRelationDialog::add_tag(){
	const uint64_t tag_id = this->win->ask_for_tag();
	if (tag_id == 0)
		return;
	
	compsky::mysql::exec(_mysql::obj, BUF, "INSERT IGNORE INTO relation2tag (relation_id, tag_id) VALUES (", this->id, ',', tag_id, ")");
}

void InstanceRelationDialog::display_hub(){
	RelationHub* hub = new RelationHub(this->win, nullptr);
	hub->exec();
	delete hub;
}
