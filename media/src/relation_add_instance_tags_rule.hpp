/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef TAGEM_MEDIA_RELATION_ADD_INSTANCE_TAG_RULE_HPP
#define TAGEM_MEDIA_RELATION_ADD_INSTANCE_TAG_RULE_HPP


#include "mainwindow.hpp"

#include <QGridLayout>
#include <QDialog>
#include <QPushButton>
#include <compsky/mysql/mysql.h>
#include <vector>


namespace rait {
	constexpr static const char* const tables[] = {
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
		RES_SLAVE,
		N_TABLES
	};
}


class RelationAddInstanceTagsRule : public QDialog {
  private:
	void add_column(const int col,  const char* const column);
	void add_tag();
	void unlink_tag();
	QGridLayout* l;
	MainWindow* const win;
	const uint64_t rule_id;
	MYSQL_RES* mysql_res;
	MYSQL_ROW mysql_row;
	QPushButton* add_btns[rait::N_TABLES];
	struct UnlinkBtn2TagId {
		const uint64_t id;
		QPushButton* const btn;
		unsigned int which_table;
		UnlinkBtn2TagId(const uint64_t _tag_id,  QPushButton* const _btn,  const unsigned int _which_table) : id(_tag_id), btn(_btn), which_table(_which_table) {}
	};
	std::vector<UnlinkBtn2TagId> unlink_tag_btns;
  public:
	explicit RelationAddInstanceTagsRule(MainWindow* const _win,  const uint64_t _rule_id,  const QString& _name,  QWidget* parent = nullptr);
};


#endif
