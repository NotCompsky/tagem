#include "relation_hub.hpp"

#include <compsky/mysql/query.hpp>

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>


namespace _mysql {
	extern MYSQL* obj;
}

extern char BUF[];


RelationHub::RelationHub(MainWindow* const _win,  QWidget* parent)
: QDialog(parent)
, win(_win)
{
	QVBoxLayout* l = new QVBoxLayout(this);
	
	l->addWidget(new QLabel("Relation Tags, Master Instance Tag, Slave Instance Tag  ->  Resultant Tag"));
	
	compsky::mysql::query(
		_mysql::obj,
		this->res,
		BUF,
		"SELECT"
			" t1.name" // as tag
			",t2.name" // as master
			",t3.name" // as slave
			",t4.name" // as result
		" "
		"FROM relationtag2tag rt2t, tag t1, tag t2, tag t3, tag t4 "
		"WHERE t1.id=rt2t.tag "
		  "AND t2.id=rt2t.master "
		  "AND t3.id=rt2t.slave "
		  "AND t4.id=rt2t.result "
	);
	char* _relation_tag;
	char* _master_instance_tag;
	char* _slave_instance_tag;
	char* _result_instance_tag;
	while(compsky::mysql::assign_next_row(this->res, &this->row, &_relation_tag, &_master_instance_tag, &_slave_instance_tag, &_result_instance_tag)){
		QHBoxLayout* hbox = new QHBoxLayout;
		
		hbox->addWidget(new QLabel(_relation_tag));
		hbox->addWidget(new QLabel(_master_instance_tag));
		hbox->addWidget(new QLabel(_slave_instance_tag));
		hbox->addWidget(new QLabel(_result_instance_tag));
		
		l->addLayout(hbox);
	}
	
	{
		QPushButton* btn = new QPushButton("+", this);
		connect(btn, &QPushButton::clicked, this, &RelationHub::add);
		l->addWidget(btn);
	}
}

void RelationHub::add(){
	const uint64_t _relation_tag        = this->win->ask_for_tag("Relation Tag");
	if (_relation_tag == 0)
		return;
	const uint64_t _master_instance_tag = this->win->ask_for_tag("Master Instance Tag");
	if (_master_instance_tag == 0)
		return;
	const uint64_t _slave_instance_tag  = this->win->ask_for_tag("Slave Instance Tag");
	if (_slave_instance_tag == 0)
		return;
	const uint64_t _result_instance_tag = this->win->ask_for_tag("Resulting Tag");
	if (_result_instance_tag == 0)
		return;
	
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"INSERT IGNORE INTO relationtag2tag "
		"(tag, master, slave, result)"
		"VALUES(",
			_relation_tag,        ',',
			_master_instance_tag, ',',
			_slave_instance_tag,  ',',
			_result_instance_tag,
		")"
	);
}
