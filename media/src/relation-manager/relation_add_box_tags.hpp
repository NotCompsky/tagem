#ifndef TAGEM_MEDIA_RELATION_ADD_INSTANCE_TAGS_HPP
#define TAGEM_MEDIA_RELATION_ADD_INSTANCE_TAGS_HPP

#include "../mainwindow.hpp"
#include <compsky/mysql/mysql.h>
#include <QDialog>
#include <QGridLayout>

#include <vector>


class MainWindow;


class RelationAddBoxTags : public QDialog {
  public:
	RelationAddBoxTags(MainWindow* const _win,  QWidget* parent = nullptr);
  private:
	struct CannotThinkOfAGoodName_sdfsdffdsdf {
		QPushButton* const name;
		QPushButton* const del;
		const uint64_t id;
		CannotThinkOfAGoodName_sdfsdffdsdf(QPushButton* const _name,  QPushButton* const _del,  const uint64_t _id)
		: name(_name)
		, del(_del)
		, id(_id)
		{}
	};
	
	void create(); // SLOT
	void add_rule(const uint64_t id,  const QString& qstr);
	void display_rule(); // SLOT
	void delete_rule(); // SLOT
	MainWindow* const win;
	QGridLayout* l;
	int row;
	std::vector<CannotThinkOfAGoodName_sdfsdffdsdf> sdfsdffdsdfs;
	MYSQL_RES* mysql_res;
	MYSQL_ROW mysql_row;
};


#endif
