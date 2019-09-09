#ifndef TAGEM_MEDIA_ERA_MANAGER_HPP
#define TAGEM_MEDIA_ERA_MANAGER_HPP

#include "../mainwindow.hpp"
#include <compsky/mysql/mysql.h>
#include <QDialog>
#include <QGridLayout>
#include <vector>


class QLabel;
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
	struct EyeCandyOfRow {
		QLabel* const tags;
		QLabel* const frame_a;
		QLabel* const frame_b;
		EyeCandyOfRow(QLabel* const _tags,  QLabel* const _frame_a,  QLabel* const _frame_b)
		: tags(_tags)
		, frame_a(_frame_a)
		, frame_b(_frame_b)
		{}
	};
	void add_era(const uint64_t id,  const uint64_t frame_a,  const uint64_t frame_b,  const char* const tags);
	void goto_era(); // SLOT
	void del_era(); // SLOT
	QPushButton* reverse_lookup(std::map<QPushButton*, Era*> map,  Era* const era_p);
	MainWindow* const win;
	QGridLayout* l;
	int row;
	MYSQL_RES* mysql_res;
	MYSQL_ROW mysql_row;
	std::vector<Era> eras;
	std::map<Era*, EyeCandyOfRow> era2eyecandy;
	std::map<QPushButton*, Era*> goto2era;
	std::map<QPushButton*, Era*> del2era;
};


#endif
