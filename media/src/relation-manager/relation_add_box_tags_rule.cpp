/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "relation_add_box_tags_rule.hpp"
#include "../add_new_tag.hpp"
#include <compsky/mysql/query.hpp>

#include <QLabel>
#include <QMessageBox>
#include <QRegExpValidator>


#define N_COLS_PER_TABLE 3


namespace _mysql {
	extern MYSQL* obj;
	extern MYSQL_RES* res1;
	extern MYSQL_ROW row1;
}


namespace _f {
	constexpr static const compsky::asciify::flag::StrLen strlen;
}

extern char BUF[];


RelationAddBoxTagsRule::RelationAddBoxTagsRule(MainWindow* const _win,  const uint64_t _rule_id,  const QString& _name,  QWidget* parent)
: QDialog(parent)
, win(_win)
, rule_id(_rule_id)
, row(0)
{
	this->setWindowTitle(_name);
	
	this->l = new QGridLayout;
	
	// this->l->addWidget(OBJECT, ROW, COL, ROW_SPAN, COL_SPAN);
	this->l->addWidget(new QLabel("[NULL] tag means that the box inherits the specified tags from the related tag"),  this->row++,  0,  1,  rait::N_TABLES * N_COLS_PER_TABLE);
	this->l->addWidget(new QLabel("The checkbox by each tag specifies if the descendant tags are included "),  this->row++,  0,  1,  rait::N_TABLES * N_COLS_PER_TABLE);
	
	int table_n = 0;
	for (const char* const table : rait::tables){
		this->add_column(table_n++, table);
	}
	
	{
		QPushButton* _btn = new QPushButton("SQL");
		connect(_btn, &QPushButton::clicked, this, &RelationAddBoxTagsRule::test);
		this->l->addWidget(_btn,  this->row++,  0);
	}
	
	this->setLayout(this->l);
}

void RelationAddBoxTagsRule::add_column(const int table_n,  const char* const table){
	int row = this->row;
	const int layout_column = table_n * N_COLS_PER_TABLE;
	this->l->addWidget(new QLabel(table),  row++,  layout_column,  1,  N_COLS_PER_TABLE);
	
	if (table_n >= N_REQ_TABLES){
		++row;
	} else {
		unsigned int _which_tbl_operator;
		compsky::mysql::query(
			_mysql::obj,
			this->mysql_res,
			BUF,
			"SELECT ", table, "_operator "
			"FROM relation_add_box_tags__rules "
			"WHERE id=", this->rule_id
		);
		while(compsky::mysql::assign_next_row(this->mysql_res, &this->mysql_row, &_which_tbl_operator)){
			this->which_tbl_operator__labels[table_n] = new QLineEdit(rait::operators[_which_tbl_operator]);
			this->which_tbl_operator__labels[table_n]->setValidator(new QRegExpValidator(QRegExp("AND|X?OR|NOT"), this));
			connect(this->which_tbl_operator__labels[table_n],  &QLineEdit::editingFinished,  this,  &RelationAddBoxTagsRule::set_table_logic_operator);
			this->l->addWidget(this->which_tbl_operator__labels[table_n],  row++,  layout_column,  1,  N_COLS_PER_TABLE);
		}
	}
	
	compsky::mysql::query(
		_mysql::obj,
		this->mysql_res,
		BUF,
		"SELECT IFNULL(t.id, 0), t.name, a.descendants_too "
		"FROM relation_add_box_tags__", table ,"_tags a "
		"LEFT JOIN tag t ON t.id=a.tag "
		"WHERE a.rule=", this->rule_id
	);
	uint64_t _tag_id;
	const char* _tag_name;
	uint8_t _descendants_too;
	while(compsky::mysql::assign_next_row(this->mysql_res, &this->mysql_row, &_tag_id, &_tag_name, &_descendants_too)){
		this->l->addWidget(new QLabel((_tag_name != nullptr) ? _tag_name : "[NULL]"),  row,  layout_column);
		
		NamedCheckBox* box = new NamedCheckBox(table, _tag_id, _descendants_too);
		connect(box, &QCheckBox::stateChanged, this, &RelationAddBoxTagsRule::tag_descendants_checkbox_changed);
		this->l->addWidget(box,  row,  layout_column + 1);
		
		QPushButton* unlink = new QPushButton("-");
		this->unlink_tag_btns.emplace_back(_tag_id, unlink, table_n);
		connect(unlink, &QPushButton::clicked, this, &RelationAddBoxTagsRule::unlink_tag);
		this->l->addWidget(unlink,  row,  layout_column + 2);
		
		++row;
	}
	
	this->add_btns[table_n] = new QPushButton("+");
	connect(this->add_btns[table_n], &QPushButton::clicked, this, &RelationAddBoxTagsRule::add_tag);
	this->l->addWidget(this->add_btns[table_n],  row,  layout_column,  1,  N_COLS_PER_TABLE);
}

void RelationAddBoxTagsRule::add_tag(){
	unsigned int i = 0;
	for (const QPushButton* btn : this->add_btns){
		if (btn != sender())
			++i;
		else
			break;
	}
	
	const uint64_t tag_id = ask_for_tag();
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"INSERT INTO relation_add_box_tags__", rait::tables[i], "_tags "
		"(rule, tag) "
		"VALUES",
		'(', this->rule_id, ',', tag_id, ')'
	);
}

void RelationAddBoxTagsRule::unlink_tag(){
	uint64_t tag_id;
	unsigned int i = 0;
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
		"DELETE FROM relation_add_box_tags__", rait::tables[i], "_tags "
		"WHERE rule=", this->rule_id,
		 " AND tag=",  tag_id
	);
}

void RelationAddBoxTagsRule::set_table_logic_operator(){
	unsigned int i = 0;
	QString qstr;
	for (const QLineEdit* x : this->which_tbl_operator__labels){
		if (x != sender())
			++i;
		else {
			qstr = QString(" ") + x->text() + QString(" ");
			break;
		}
	}
	
	unsigned int which_tbl_operator = 0;
	for (const char* const s : rait::operators){
		if (qstr != s)
			++which_tbl_operator;
		else
			break;
	}
	if (which_tbl_operator == rait::tbl_operator::N)
		// i.e. s is not equal to any operator
		// i.e. s IN ("", "AN", "A", "O", "XO", "X", "NO", "N")
		return;
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"UPDATE relation_add_box_tags__rules "
		"SET ", rait::tables[i], "_operator=", which_tbl_operator, " "
		"WHERE id=", this->rule_id
	);
}

void RelationAddBoxTagsRule::tag_descendants_checkbox_changed(int state){
	const NamedCheckBox* box = static_cast<const NamedCheckBox*>(sender());
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"UPDATE relation_add_box_tags__", box->table, "_tags "
		"SET "
			"descendants_too=",
				(box->isChecked()) ? '1' : '0', " "
		"WHERE rule=", this->rule_id, " "
		  "AND tag=", box->tag
	);
}
#include <stdio.h>
void RelationAddBoxTagsRule::compile(){
	char* _compiled_init = this->compiled_init;
	char* _compiled_tbls = this->compiled_tbls;
	char* _compiled_fltr = this->compiled_fltr;
	char* _compiled_hvng = this->compiled_hvng;
	
	struct Bertrand {
		const char* const tbl;
		const char* const thing2tag_alias;
	};
	
	constexpr
	static const Bertrand bertrands[3] = {
		{"relation", "r2t"},
		{"master",   "i2m_master"},
		{"slave",    "i2m_slave"}
	};
	
	for (const Bertrand bertrand : bertrands){
		compsky::mysql::query(
			_mysql::obj,
			this->mysql_res,
			BUF,
			"SELECT "
				"r.req_", bertrand.tbl, "_operator,"
				"GROUP_CONCAT(DISTINCT CONCAT(A.tag, ',', A.descendants_too)) " // AS 'Req. Relation'
			"FROM relation_add_box_tags__rules r, relation_add_box_tags__req_", bertrand.tbl, "_tags A "
			"WHERE A.rule=r.id "
			  "AND r.id=", this->rule_id, " "
			"HAVING SUM(A.tag)" // != 0
		);
		unsigned int _operator_id;
		const char* _tags;
		while(compsky::mysql::assign_next_row(this->mysql_res, &this->mysql_row,
			&_operator_id,
			&_tags
		)){
			// Only one iteration
			
			const char* const _operator = rait::operators[_operator_id];
			{
				// GROUP_CONCAT(ORDERED ...) = _tags   ???
				const char* _itr = _tags;
				unsigned int i = 0;
				char* const _compiled_fltr__before_preloop = _compiled_fltr;
				compsky::asciify::asciify(
					_compiled_fltr,
					" AND ", bertrand.thing2tag_alias, ".tag_id IN ("
				);
				char* const _compiled_hvng__before_preloop = _compiled_hvng;
				compsky::asciify::asciify(
					_compiled_hvng,
					" AND ("
				);
				char* const _compiled_fltr__before_loop = _compiled_fltr;
				constexpr static const char* const _union_string = " UNION ";
				while(true){
					if(*_itr == ','){
						const bool _descendants_too = (_itr[1] == '1');
						if (_descendants_too){
							compsky::asciify::asciify(
								_compiled_init,
								"CALL descendant_tags_id_from_ids('_", bertrand.tbl, ++i, "', '",
									_f::strlen,  _tags,  (uintptr_t)_itr - (uintptr_t)_tags,
								"');"
							);
							compsky::asciify::asciify(
								_compiled_tbls,
								'_', bertrand.tbl, i, ','
							);
							compsky::asciify::asciify(
								// Shouldn't be a need for IN filter for AND operators - but the database doesn't optimise it properly without.
								_compiled_fltr,
								"SELECT node FROM _", bertrand.tbl, i, _union_string
							);
							compsky::asciify::asciify(
								_compiled_hvng,
								"SUM(", bertrand.thing2tag_alias, ".tag_id=_", bertrand.tbl, i, ".node) ", _operator // i.e. != 0
							);
						} else {
							compsky::asciify::asciify(
								_compiled_fltr,
								"SELECT ",
									_f::strlen,  _tags,  (uintptr_t)_itr - (uintptr_t)_tags,
								_union_string
							);
							compsky::asciify::asciify(
								_compiled_hvng,
								"SUM(", bertrand.thing2tag_alias, ".tag_id=",
									_f::strlen,  _tags,  (uintptr_t)_itr - (uintptr_t)_tags,
								")", _operator // i.e. != 0
							);
						}
						if(_itr[2] == 0)
							break;
						_itr += 2; // Skip _descendants_too char and its separator
						_tags = _itr+1;
					}
					++_itr;
				}
				if (_compiled_fltr == _compiled_fltr__before_loop){
					// Did not enter the loop
					_compiled_fltr = _compiled_fltr__before_preloop;
					_compiled_hvng = _compiled_hvng__before_preloop;
				} else {
					_compiled_fltr -= std::char_traits<char>::length(_union_string);
					_compiled_hvng -= strlen(_operator);
					compsky::asciify::asciify(
						_compiled_fltr,
						")"
					);
					compsky::asciify::asciify(
						_compiled_hvng,
						")"
					);
				}
			}
		}
	}
	
	compsky::asciify::asciify(_compiled_init, '\0');
	compsky::asciify::asciify(_compiled_tbls, '\0');
	compsky::asciify::asciify(_compiled_fltr, '\0');
	compsky::asciify::asciify(_compiled_hvng, '\0');
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"UPDATE relation_add_box_tags__rules r "
		"SET "
			"compiled_init=\"", _f::strlen, this->compiled_init, (uintptr_t)_compiled_init - (uintptr_t)compiled_init, "\","
			"compiled_tbls=\"", _f::strlen, this->compiled_tbls, (uintptr_t)_compiled_tbls - (uintptr_t)compiled_tbls, "\","
			"compiled_fltr=\"", _f::strlen, this->compiled_fltr, (uintptr_t)_compiled_fltr - (uintptr_t)compiled_fltr, "\","
			"compiled_hvng=\"", _f::strlen, this->compiled_hvng, (uintptr_t)_compiled_hvng - (uintptr_t)compiled_hvng, "\" "
		"WHERE r.id=", this->rule_id
	);
}

void RelationAddBoxTagsRule::test(){
	this->compile();
	
	char* itr = BUF;
	
	compsky::asciify::asciify(
		itr,
		
		this->compiled_init,
		
		"SELECT "
			"f.name "
		"FROM ",
			this->compiled_tbls,
			"file f,"
			"relation r,"
			"relation2tag r2t,"
			"box b_master,"
			"box2tag b2m_master,"
			"box2tag b2m_slave "
		"WHERE r.id=r2t.relation_id "
		  "AND b_master.id=b2m_master.box_id "
		  "AND f.id=b_master.file_id "
		  "AND b2m_master.box_id=r.master_id "
		  "AND i2m_slave.box_id=r.slave_id ",
		  this->compiled_fltr,
		"GROUP BY r.id "
		"HAVING TRUE ",
		   this->compiled_hvng,
		'\0'
	);
	QMessageBox::information(this, "SQL", BUF);
}

void RelationAddBoxTagsRule::reject(){
	// Intercepts close (via the 'X' button) events
	this->compile();
	
	QDialog::reject();
}
