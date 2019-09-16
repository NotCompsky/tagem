#pragma once


#include <QDialog>
#include <QPushButton>
#include <vector>


class Menu : public QDialog {
private:
	std::vector<QPushButton*> btns;
	void option_chosen();
public:
	Menu();
};
