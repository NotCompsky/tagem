#include "box_relation_dialog.hpp"
#include "../unlink_tag_btn.hpp"
#include "../name_dialog.hpp"
#include "../add_new_tag.hpp"

#include <compsky/mysql/query.hpp>

#include <QLabel>
#include <QGridLayout>
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


BoxRelationDialog::BoxRelationDialog(const uint64_t _id,  MainWindow* const _win,  QWidget* parent)
: QDialog(parent)
, id(_id)
, win(_win)
{
	QVBoxLayout* l = new QVBoxLayout(this);
	
	{
		QGridLayout* grid = new QGridLayout;
		
		auto lambda__thingymajig = [&_id, &grid](const int _col,  const char* const s){
			int _row = 0;
			QLabel* lbl = new QLabel(s);
			grid->addWidget(lbl, _row++, _col);
			lbl->setStyleSheet("font-weight: bold");
			
			compsky::mysql::query(
				_mysql::obj,
				RES2,
				BUF,
				"SELECT t.name "
				"FROM tag t, box2tag b2t, box b, relation r "
				"WHERE b.id=r.", s, "_id "
				"AND b2t.box_id=b.id "
				"AND t.id=b2t.tag_id "
				"AND r.id=", _id
			);
			const char* _tag_name;
			while(compsky::mysql::assign_next_row(RES2, &ROW2, &_tag_name)){
				
				
				grid->addWidget(new QLabel(_tag_name), _row++, _col);
			}
		};
		
		lambda__thingymajig(0, "master");
		lambda__thingymajig(1, "slave");
		
		l->addLayout(grid);
	}
	
	
	l->addWidget(new QLabel("Tags"));
	compsky::mysql::query(_mysql::obj,  RES2,  BUF,  "SELECT t.id, t.name FROM tag t, relation2tag r2t WHERE t.id=r2t.tag_id AND r2t.relation_id=", _id);
	uint64_t _tag_id;
	const char* _tag_name;
	while(compsky::mysql::assign_next_row(RES2, &ROW2, &_tag_id, &_tag_name)){
		QHBoxLayout* hbox = new QHBoxLayout;
		
		hbox->addWidget(new QLabel(_tag_name));
		hbox->addWidget(new UnlinkTagBtn("DELETE FROM relation2tag WHERE relation_id=", _id, " AND tag_id=", _tag_id, this));
		
		l->addLayout(hbox);
	}
	
	{
		QPushButton* btn = new QPushButton("+Tag", this);
		connect(btn, &QPushButton::clicked, this, &BoxRelationDialog::add_tag);
		l->addWidget(btn);
	}
	
	{
		QPushButton* btn = new QPushButton("Hub", this);
		connect(btn, &QPushButton::clicked, this->win, &MainWindow::display_relation_hub);
		l->addWidget(btn);
	}
}

void BoxRelationDialog::add_tag(){
	const uint64_t tag_id = ask_for_tag();
	if (tag_id == 0)
		return;
	
	compsky::mysql::exec(_mysql::obj, BUF, "INSERT IGNORE INTO relation2tag (relation_id, tag_id) VALUES (", this->id, ',', tag_id, ")");
}
