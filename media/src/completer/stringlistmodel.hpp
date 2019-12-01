/*
 * CC BY-SA 4.0
 * 
 * Original code is copyright Marek R (https://stackoverflow.com/users/1387438/marek-r) and was viewed from https://stackoverflow.com/questions/31465850/a-qlineedit-qcombobox-search-that-ignores-diacritics
 */


#pragma once
#include <QStringListModel>
#include <QVariant>
#include "simplify_str.hpp"


class StringListModel : public QStringListModel {
	int mDiactricFreeRole;
	
	StringListModel(QObject* parent)
		: QStringListModel(parent)
		, mDiactricFreeRole(Qt::UserRole + 10)
	{}

	QVariant data(const QModelIndex& index,  int role) const {
		if (role==this->mDiactricFreeRole) {
			QString value = QStringListModel::data(index, Qt::DisplayRole).toString();
			return simplify_str(value);
		} else {
			return QStringListModel::data(index, role);
		}
	}
};
