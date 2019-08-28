/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "relation_add_instance_tags_rule.hpp"

#include <compsky/mysql/query.hpp>

#include <QCheckBox>
#include <QLabel>


#define N_COLS_PER_TABLE 3


namespace _mysql {
	extern MYSQL* obj;
}

extern char BUF[];

constexpr static const char* const tables[] = {
	"req_master",
	"req_slave",
	"res_master",
	"res_slave"
};

enum {
	REQ_MASTER,
	REQ_SLAVE,
	RES_MASTER,
	RES_SLAVE,
	N_TABLES
};


RelationAddInstanceTagsRule::RelationAddInstanceTagsRule(MainWindow* const _win,  const uint64_t _rule_id,  const QString& _name,  QWidget* parent)
: QDialog(parent)
, win(_win)
, rule_id(_rule_id)
{
	this->l = new QGridLayout;
	
	// this->l->addWidget(OBJECT, ROW, COL, ROW_SPAN, COL_SPAN);
	this->l->addWidget(new QLabel(_name),  0,  0,  1,  N_TABLES * N_COLS_PER_TABLE);
	this->l->addWidget(new QLabel("[NULL] tag means that the instance inherits the specified tags from the related instance"),  1,  0,  1,  N_TABLES * N_COLS_PER_TABLE);
	
	int table_n = 0;
	for (const char* const table : tables){
		this->add_column(table_n++, table);
	}
	
	this->setLayout(this->l);
}

void RelationAddInstanceTagsRule::add_column(const int table_n,  const char* const table){
	int row = 2;
	const int layout_column = table_n * N_COLS_PER_TABLE;
	this->l->addWidget(new QLabel(table),  row++,  layout_column,  1,  N_COLS_PER_TABLE);
	compsky::mysql::query(
		_mysql::obj,
		this->mysql_res,
		BUF,
		"SELECT IFNULL(t.id, 0), t.name, a.descendants_too "
		"FROM relation_add_instance_tags__", table ,"_tags a "
		"LEFT JOIN tag t ON t.id=a.tag "
		"WHERE a.rule=", this->rule_id
	);
	uint64_t _tag_id;
	char* _tag_name;
	uint8_t _descendants_too;
	while(compsky::mysql::assign_next_row(this->mysql_res, &this->mysql_row, &_tag_id, &_tag_name, &_descendants_too)){
		this->l->addWidget(new QLabel((_tag_name != nullptr) ? _tag_name : "[NULL]"),  row,  layout_column);
		
		QCheckBox* box = new QCheckBox;
		box->setChecked(_descendants_too);
		if (_tag_name == nullptr)
			box->setEnabled(false);
		this->l->addWidget(box,  row,  layout_column + 1);
		
		QPushButton* unlink = new QPushButton("-");
		this->unlink_tag_btns.emplace_back(_tag_id, unlink, table_n);
		connect(unlink, &QPushButton::clicked, this, &RelationAddInstanceTagsRule::unlink_tag);
		this->l->addWidget(unlink,  row,  layout_column + 2);
		
		++row;
	}
	
	this->add_btns[table_n] = new QPushButton("+");
	connect(this->add_btns[table_n], &QPushButton::clicked, this, &RelationAddInstanceTagsRule::add_tag);
	this->l->addWidget(this->add_btns[table_n],  row,  layout_column,  1,  N_COLS_PER_TABLE);
}

void RelationAddInstanceTagsRule::add_tag(){
	unsigned int i;
	for (const QPushButton* btn : this->add_btns){
		if (btn != sender())
			++i;
		else
			break;
	}
	
	const uint64_t tag_id = this->win->ask_for_tag();
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"INSERT INTO relation_add_instance_tags__", tables[i], "_tags "
		"(rule, tag) "
		"VALUES",
		'(', this->rule_id, ',', tag_id, ')'
	);
}

void RelationAddInstanceTagsRule::unlink_tag(){
	uint64_t tag_id;
	unsigned int i;
	for (const UnlinkBtn2TagId x : this->unlink_tag_btns){
		if (x.btn == sender()){
			tag_id = x.id;
			i = x.which_table;
			delete x.btn;
			//x.btn = nullptr;
			break;
		}
	}
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"DELETE FROM relation_add_instance_tags__", tables[i], "_tags "
		"WHERE rule=", this->rule_id,
		 " AND tag=",  tag_id
	);
	
	
}
