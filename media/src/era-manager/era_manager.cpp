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
	const uint64_t n_results = compsky::mysql::n_results<uint64_t>(this->mysql_res);
	this->eras.reserve(n_results);
	uint64_t _id;
	uint64_t _frame_a;
	uint64_t _frame_b;
	const char* _tags; // TODO: Placeholder
	while(compsky::mysql::assign_next_row(this->mysql_res, &this->mysql_row, &_id, &_frame_a, &_frame_b, &_tags)){
		this->add_era(_id, _frame_a, _frame_b, _tags);
	}
}

void EraManager::add_era(const uint64_t id,  const uint64_t frame_a,  const uint64_t frame_b,  const char* const tags){
	this->eras.emplace_back(id, frame_a, frame_b);
	Era* const era_p = &this->eras[this->eras.size()-1];
	
	QLabel* tag_label = new QLabel(tags);
	QLabel* frame_a_label = new QLabel(QString::number((double)frame_a/(double)this->win->m_player->duration()));
	QLabel* frame_b_label = new QLabel(QString::number((double)frame_b/(double)this->win->m_player->duration()));
	this->l->addWidget(tag_label,      this->row,  0);
	this->l->addWidget(frame_a_label,  this->row,  1);
	this->l->addWidget(frame_b_label,  this->row,  2);
	this->era2eyecandy.try_emplace(era_p, tag_label, frame_a_label, frame_b_label);
	
	QPushButton* btn = new QPushButton("Goto");
	connect(btn, &QPushButton::clicked, this, &EraManager::goto_era);
	this->goto2era.try_emplace(btn, era_p);
	this->l->addWidget(btn, this->row, 3);
	
	{
		QPushButton* _btn = new QPushButton("-");
		connect(_btn, &QPushButton::clicked, this, &EraManager::del_era);
		this->del2era.try_emplace(_btn, era_p);
		this->l->addWidget(_btn, this->row, 4);
	}
	
	++this->row;
}

void EraManager::goto_era(){
	this->win->m_player->seek((qint64)(*this->goto2era.at(static_cast<QPushButton*>(sender()))).frame_a);
}

QPushButton* EraManager::reverse_lookup(std::map<QPushButton*, Era*> map,  Era* const era_p){
	for (const auto paar : map)
		if (paar.second == era_p)
			return paar.first;
}

void EraManager::del_era(){
	Era* const era_p = this->del2era.at(static_cast<QPushButton*>(sender()));
	// No need (atm) to set era.id = 0;
	delete this->reverse_lookup(this->goto2era, era_p);
	delete sender();
	EyeCandyOfRow ecod = this->era2eyecandy.at(era_p);
	delete ecod.tags;
	delete ecod.frame_a;
	delete ecod.frame_b;
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"DELETE FROM era WHERE id=", era_p->id
	);
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"DELETE FROM era2tag WHERE era_id=", era_p->id
	);
}
