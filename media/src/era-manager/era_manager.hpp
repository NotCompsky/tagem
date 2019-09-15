#ifndef TAGEM_MEDIA_ERA_MANAGER_HPP
#define TAGEM_MEDIA_ERA_MANAGER_HPP

#include "../era.hpp"
#include <compsky/mysql/mysql.h>
#include <QDialog>
#include <QGridLayout>
#include <vector>


class MainWindow;
class QComboBox;
class QLabel;
class QPushButton;


class EraManager : public QDialog {
  public:
	EraManager(MainWindow* const _win,  QWidget* parent = nullptr);
  private:
	void toggle_display_eras(); // SLOT
	void add_era(Era* const era);
	void goto_era(); // SLOT
	void del_era(); // SLOT
	void change_start_method_name(int indx); // SLOT
	void change_end_method_name(int indx); // SLOT
	void edit_python_script(); // SLOT
	QObject* reverse_lookup(Era* const era_p);
	MainWindow* const win;
	QGridLayout* l;
	int row;
	MYSQL_RES* mysql_res;
	MYSQL_ROW mysql_row;
	std::map<QObject*, Era*> qobj2era;
};


#endif
