#include "era_manager.hpp"
#include "../overlay.hpp"
#include "../mainwindow.hpp"
#include "../menu.hpp"
#include <compsky/mysql/query.hpp>
#include <QComboBox>
#include <QInputDialog>
#include <QPushButton>


namespace _mysql {
	extern MYSQL* obj;
}

namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
}

extern MYSQL_RES* RES1;
extern MYSQL_ROW  ROW1;
extern char BUF[];


EraManager::EraManager(MainWindow* const _win,  QWidget* parent)
: QDialog(parent)
, win(_win)
, row(0)
{
	this->setWindowTitle("Eras");
	
	this->l = new QGridLayout(this);
	
	{
		QCheckBox* b = new QCheckBox("Display?");
		b->setChecked(this->win->are_eras_visible);
		connect(b, &QCheckBox::toggled, this, &EraManager::toggle_display_eras);
		this->l->addWidget(b);
		++this->row;
	}
	
	for (Era& era : this->win->eras){
		this->add_era(&era);
	}
}

void EraManager::toggle_display_eras(){
	this->win->are_eras_visible = !this->win->are_eras_visible;
	this->win->main_widget_overlay->repaint();
}

void EraManager::add_era(Era* const era_p){
	compsky::mysql::query(
		_mysql::obj,
		RES1,
		BUF,
		"SELECT t.name FROM era2tag e2t, tag t WHERE t.id=e2t.tag_id AND e2t.era_id=", era_p->id
	);
	QString tags = ""; // TMP
	const char* _tag_name;
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &_tag_name)){
		tags += _tag_name;
		tags += ", ";
	}
	QLabel* tag_label = new QLabel(tags);
	
	QLabel* frame_a_label = new QLabel(QString::number((double)era_p->start/(double)this->win->m_player->duration()));
	QLabel* frame_b_label = new QLabel(QString::number((double)era_p->end  /(double)this->win->m_player->duration()));
	this->l->addWidget(tag_label,      this->row,  0);
	this->l->addWidget(frame_a_label,  this->row,  1);
	this->l->addWidget(frame_b_label,  this->row,  2);
	this->qobj2era.try_emplace(tag_label, era_p);
	this->qobj2era.try_emplace(frame_a_label, era_p);
	this->qobj2era.try_emplace(frame_b_label, era_p);
	
	QPushButton* btn = new QPushButton("Goto");
	connect(btn, &QPushButton::clicked, this, &EraManager::goto_era);
	this->qobj2era.try_emplace(btn, era_p);
	this->l->addWidget(btn, this->row, 3);
	
	{
		// Start method
		QComboBox* _dropdown = new QComboBox();
		_dropdown->addItems(this->win->qmethod_names);
		_dropdown->setCurrentIndex(era_p->start_method);
		connect(_dropdown, qOverload<int>(&QComboBox::currentIndexChanged), this, &EraManager::change_start_method_name);
		this->qobj2era.try_emplace(_dropdown, era_p);
		this->l->addWidget(_dropdown, this->row, 4);
	}
	{
		// End method
		QComboBox* _dropdown = new QComboBox();
		_dropdown->addItems(this->win->qmethod_names);
		_dropdown->setCurrentIndex(era_p->end_method);
		connect(_dropdown, qOverload<int>(&QComboBox::currentIndexChanged), this, &EraManager::change_end_method_name);
		this->qobj2era.try_emplace(_dropdown, era_p);
		this->l->addWidget(_dropdown, this->row, 5);
	}
	
	{
		QPushButton* _btn = new QPushButton("Python");
		connect(_btn, &QPushButton::clicked, this, &EraManager::edit_python_script);
		this->qobj2era.try_emplace(_btn, era_p);
		this->l->addWidget(_btn, this->row, 6);
	}
	
	{
		QPushButton* _btn = new QPushButton("-");
		connect(_btn, &QPushButton::clicked, this, &EraManager::del_era);
		this->qobj2era.try_emplace(_btn, era_p);
		this->l->addWidget(_btn, this->row, 7);
	}
	
	++this->row;
}

void EraManager::goto_era(){
	this->win->m_player->seek((qint64)(*this->qobj2era.at(sender())).start);
}

QObject* EraManager::reverse_lookup(Era* const era_p){
	for (const auto paar : this->qobj2era)
		if (paar.second == era_p)
			return paar.first;
	return nullptr;
};

void EraManager::del_era(){
	Era* const era_p = this->qobj2era.at(sender());
	// No need (atm) to set era.id = 0;
	QObject* o;
	while((o = this->reverse_lookup(era_p))){
		this->qobj2era[o] = nullptr;
		delete o;
	}
	
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

void EraManager::change_start_method_name(int indx){
	Era* era_p = this->qobj2era[sender()];
	era_p->start_method = indx;
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"UPDATE era SET start_method_id=", indx, " WHERE id=", era_p->id
	);
}

void EraManager::change_end_method_name(int indx){
	Era* era_p = this->qobj2era[sender()];
	era_p->end_method = indx;
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"UPDATE era SET end_method_id=", indx, " WHERE id=", era_p->id
	);
}

void EraManager::edit_python_script(){
	Era* era_p = this->qobj2era[sender()];
	compsky::mysql::query(
		_mysql::obj,
		RES1,
		BUF,
		"SELECT s "
		"FROM era "
		"WHERE id=", era_p->id
	);
	const char* prev_s;
	while(compsky::mysql::assign_next_row__no_free(RES1, &ROW1, &prev_s));
	bool ok;
	const QString s = QInputDialog::getMultiLineText(this, "Python Script", "Python Script", prev_s, &ok);
	if (ok){
		compsky::mysql::exec(
			_mysql::obj,
			BUF,
			"UPDATE era "
			"SET s=\"",
				_f::esc, '"', s,
			"\" WHERE id=", era_p->id
		);
	}
	mysql_free_result(RES1);
}
