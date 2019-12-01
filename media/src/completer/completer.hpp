/*
 * CC BY-SA 4.0
 * 
 * Original code is copyright Marek R (https://stackoverflow.com/users/1387438/marek-r) and was viewed from https://stackoverflow.com/questions/31465850/a-qlineedit-qcombobox-search-that-ignores-diacritics
 */


#pragma once
#include <QCompleter>
#include "simplify_str.hpp"


class Completer : public QCompleter {
public:
	Completer(QStringList& l,  QObject* parent = nullptr)
		: QCompleter(l, parent)
	{}

private:
	QStringList splitPath(const QString &path) const {
		return QStringList() << simplify_str(path);
	}

	QString pathFromIndex(const QModelIndex &index) const {
		// Need to use original value when value is selected
		return index.data().toString();
	}
};
