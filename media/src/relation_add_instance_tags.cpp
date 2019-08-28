#include "relation_add_instance_tags.hpp"
#include "relation_add_instance_tags_rule.hpp"

#include <compsky/mysql/query.hpp>

#include <QInputDialog>
#include <QPushButton>


namespace _mysql {
	extern MYSQL* obj;
}

namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
}

constexpr static const char* const columns[] = {
	"req_master",
	"req_slave",
	"res_master",
	"res_slave"
};

enum {
	REQ_MASTER,
	REQ_SLAVE,
	RES_MASTER,
	RES_SLAVE
};

extern char BUF[];


RelationAddInstanceTags::RelationAddInstanceTags(MainWindow* const _win,  QWidget* parent)
: QDialog(parent)
, win(_win)
, row(0)
{
	this->l = new QGridLayout(this);
	
	compsky::mysql::query_buffer(
		_mysql::obj,
		this->mysql_res,
		"SELECT id, name "
		"FROM relation_add_instance_tags__rules"
	);
	uint64_t _id;
	char* _name;
	while(compsky::mysql::assign_next_row(this->mysql_res, &this->mysql_row, &_id, &_name)){
		this->add_rule(_id, _name);
	}
	
	{
		QPushButton* btn = new QPushButton("+", this);
		connect(btn, &QPushButton::clicked, this, &RelationAddInstanceTags::create);
		this->l->addWidget(btn, this->row++, 0);
	}
}

void RelationAddInstanceTags::add_rule(const uint64_t id,  const QString& qstr){
	QPushButton* name = new QPushButton(qstr);
	QPushButton* del  = new QPushButton("-");
	
	this->sdfsdffdsdfs.emplace_back(name, del, id);
	
	connect(name, &QPushButton::clicked, this, &RelationAddInstanceTags::display_rule);
	connect(del,  &QPushButton::clicked, this, &RelationAddInstanceTags::delete_rule);
	
	this->l->addWidget(name,  this->row,  0);
	this->l->addWidget(del,   this->row,  1);
	++this->row;
}

void RelationAddInstanceTags::create(){
	const QString _name = QInputDialog::getText(this,  "Tag Name",  "");
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"INSERT INTO relation_add_instance_tags__rules "
		"(name)"
		"VALUES(\"",
			_f::esc, '"', _name,
		"\")"
	);
	
	uint64_t _id;
	compsky::mysql::query(
		_mysql::obj,
		this->mysql_res,
		BUF,
		"SELECT id "
		"FROM relation_add_instance_tags__rules "
		"WHERE name=\"", _f::esc, '"', _name, "\""
	);
	while(compsky::mysql::assign_next_row(this->mysql_res, &this->mysql_row, &_id)){
		this->add_rule(_id, _name);
	}
}

void RelationAddInstanceTags::display_rule(){
	uint64_t _id;
	QPushButton* _name_btn;
	for (const auto sdfsdffdsdf : this->sdfsdffdsdfs){
		if (sdfsdffdsdf.name == sender()){
			_id = sdfsdffdsdf.id;
			_name_btn = sdfsdffdsdf.name;
			break;
		}
	}
	
	RelationAddInstanceTagsRule* dialog = new RelationAddInstanceTagsRule(this->win, _id, _name_btn->text());
	dialog->exec();
	delete dialog;
}

void RelationAddInstanceTags::delete_rule(){
	uint64_t _id;
	for (const auto sdfsdffdsdf : this->sdfsdffdsdfs){
		if (sdfsdffdsdf.del == sender()){
			_id = sdfsdffdsdf.id;
			break;
		}
	}
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"DELETE FROM relation_add_instance_tags__rules WHERE id=", _id
	);
	
	for (const char* const column : columns){
		compsky::mysql::exec(
			_mysql::obj,
			BUF,
			"DELETE FROM relation_add_instance_tags__", column, "_tags WHERE rule=", _id
		);
	}
}
