#include "relation_add_box_tags.hpp"
#include "relation_add_box_tags_rule.hpp"

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
	"req_relation",
	"req_master",
	"req_slave",
	"res_relation",
	"res_master",
	"res_slave"
};

enum {
	REQ_RELATION,
	REQ_MASTER,
	REQ_SLAVE,
	RES_RELATION,
	RES_MASTER,
	RES_SLAVE
};

extern char BUF[];


RelationAddBoxTags::RelationAddBoxTags(MainWindow* const _win,  QWidget* parent)
: QDialog(parent)
, win(_win)
, row(0)
{
	this->setWindowTitle("RAIT");
	
	this->l = new QGridLayout(this);
	
	compsky::mysql::query_buffer(
		_mysql::obj,
		this->mysql_res,
		"SELECT id, name "
		"FROM relation_add_box_tags__rules"
	);
	uint64_t _id;
	const char* _name;
	while(compsky::mysql::assign_next_row(this->mysql_res, &this->mysql_row, &_id, &_name)){
		this->add_rule(_id, _name);
	}
	
	{
		QPushButton* btn = new QPushButton("+", this);
		connect(btn, &QPushButton::clicked, this, &RelationAddBoxTags::create);
		this->l->addWidget(btn, this->row++, 0);
	}
}

void RelationAddBoxTags::add_rule(const uint64_t id,  const QString& qstr){
	QPushButton* name = new QPushButton(qstr);
	QPushButton* del  = new QPushButton("-");
	
	this->sdfsdffdsdfs.emplace_back(name, del, id);
	
	connect(name, &QPushButton::clicked, this, &RelationAddBoxTags::display_rule);
	connect(del,  &QPushButton::clicked, this, &RelationAddBoxTags::delete_rule);
	
	this->l->addWidget(name,  this->row,  0);
	this->l->addWidget(del,   this->row,  1);
	++this->row;
}

void RelationAddBoxTags::create(){
	const QString _name = QInputDialog::getText(this,  "Rule Name",  "");
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"INSERT INTO relation_add_box_tags__rules "
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
		"FROM relation_add_box_tags__rules "
		"WHERE name=\"", _f::esc, '"', _name, "\""
	);
	while(compsky::mysql::assign_next_row(this->mysql_res, &this->mysql_row, &_id)){
		this->add_rule(_id, _name);
	}
}

void RelationAddBoxTags::display_rule(){
	uint64_t _id;
	QPushButton* _name_btn;
	for (const auto sdfsdffdsdf : this->sdfsdffdsdfs){
		if (sdfsdffdsdf.name == sender()){
			_id = sdfsdffdsdf.id;
			_name_btn = sdfsdffdsdf.name;
			break;
		}
	}
	
	RelationAddBoxTagsRule* dialog = new RelationAddBoxTagsRule(this->win, _id, _name_btn->text());
	dialog->exec();
	delete dialog;
}

void RelationAddBoxTags::delete_rule(){
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
		"DELETE FROM relation_add_box_tags__rules WHERE id=", _id
	);
	
	for (const char* const column : columns){
		compsky::mysql::exec(
			_mysql::obj,
			BUF,
			"DELETE FROM relation_add_box_tags__", column, "_tags WHERE rule=", _id
		);
	}
}
