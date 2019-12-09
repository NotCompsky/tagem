/*
 * CC BY-SA 4.0
 * 
 * Original code is copyright Marek R (https://stackoverflow.com/users/1387438/marek-r) and was viewed from https://stackoverflow.com/questions/31465850/a-qlineedit-qcombobox-search-that-ignores-diacritics
 */


#pragma once
#include <QCompleter>
#include "simplify_str.hpp"
#include <QDebug>


class Completer {
public:
	QStringList stringlist;
private:
	void populate(QStringList& _stringlist){
		this->stringlist.clear();
		this->str2index.clear();
		for (auto i = 0;  i < _stringlist.size();  ++i){
			const QString qstr = _stringlist.at(i);
			this->str2index[qstr] = i;
			this->stringlist << qstr;
		}
		for (auto i = 0;  i < _stringlist.size();  ++i){
			const QString qstr = _stringlist.at(i);
			
			QString s1;
			simplify_str(qstr, s1);
			if (s1 != qstr){
				this->str2index[s1] = i;
				this->stringlist << s1;
			}
		}
	}
	void new_completer(){
		this->completer = new QCompleter(this->stringlist);
	}
public:
	std::map<QString,  unsigned int> str2index;
	QCompleter* completer;
	
	Completer()
	{
		this->new_completer();
	}
	
	~Completer(){
		delete this->completer;
	}
	
	void reset(QStringList& _stringlist){
		delete this->completer;
		this->populate(_stringlist);
		this->new_completer();
	};
	
	QString get_orig_str(const QString& s) const {
		try {
			const auto i = this->str2index.at(s);
			return this->stringlist.at(i);
		} catch (const std::out_of_range&){
			return s;
		}
	}
};
