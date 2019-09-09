#include "era_manager.hpp"
#include <compsky/mysql/query.hpp>
#include <QPushButton>


namespace _mysql {
	extern MYSQL* obj;
}

extern char BUF[];


EraManager::EraManager(MainWindow* const _win,  QWidget* parent)
: QDialog(parent)
, win(_win)
, row(0)
{
	this->setWindowTitle("Eras");
	
	this->l = new QGridLayout(this);
	
	compsky::mysql::query(
		_mysql::obj,
		this->mysql_res,
		BUF,
		"SELECT e.id, e.frame_a, e.frame_b, GROUP_CONCAT(t.name) "
		"FROM era e, era2tag e2t, tag t "
		"WHERE e2t.era_id=e.id "
		  "AND t.id=e2t.tag_id "
		  "AND file_id=", this->win->file_id, " "
		"GROUP BY e.id"
	);
	uint64_t _id;
	uint64_t _frame_a;
	uint64_t _frame_b;
	const char* _tags; // TODO: Placeholder
	while(compsky::mysql::assign_next_row(this->mysql_res, &this->mysql_row, &_id, &_frame_a, &_frame_b, &_tags)){
		this->add_era(_id, _frame_a, _frame_b, _tags);
	}
}

void EraManager::add_era(const uint64_t id,  const uint64_t frame_a,  const uint64_t frame_b,  const char* const tags){
	this->l->addWidget(new QLabel(tags),  this->row,  0);
	this->l->addWidget(new QLabel(QString::number((double)frame_a/(double)this->win->m_player->duration())),   this->row,  1);
	this->l->addWidget(new QLabel(QString::number((double)frame_b/(double)this->win->m_player->duration())),   this->row,  2);
	
	QPushButton* btn = new QPushButton("Goto");
	connect(btn, &QPushButton::clicked, this, &EraManager::goto_era);
	this->goto2era.try_emplace(btn, id, frame_a, frame_b);
	this->l->addWidget(btn, this->row, 3);
	
	++this->row;
}

void EraManager::goto_era(){
	this->win->m_player->seek((qint64)this->goto2era.at(static_cast<QPushButton*>(sender())).frame_a);
}
