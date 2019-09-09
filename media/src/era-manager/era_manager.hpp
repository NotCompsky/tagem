#ifndef TAGEM_MEDIA_ERA_MANAGER_HPP
#define TAGEM_MEDIA_ERA_MANAGER_HPP

#include "../mainwindow.hpp"
#include <compsky/mysql/mysql.h>
#include <QDialog>
#include <QGridLayout>
#include <vector>


class QPushButton;


class EraManager : public QDialog {
  public:
	EraManager(MainWindow* const _win,  QWidget* parent = nullptr);
  private:
	struct Era {
		const uint64_t id;
		const uint64_t frame_a;
		const uint64_t frame_b;
		Era(const uint64_t _id,  const uint64_t _frame_a,  const uint64_t _frame_b)
		: id(_id)
		, frame_a(_frame_a)
		, frame_b(_frame_b)
		{}
	};
	void add_era(const uint64_t id,  const uint64_t frame_a,  const uint64_t frame_b,  const char* const tags);
	void goto_era(); // SLOT
	MainWindow* const win;
	QGridLayout* l;
	int row;
	MYSQL_RES* mysql_res;
	MYSQL_ROW mysql_row;
	std::map<QPushButton*, Era> goto2era;
};


#endif
