#include "primaryitem.hpp"
#include "../add_new_tag.hpp"
#include "tagtreeview.hpp"

#include <QCompleter>
#include <QStandardItem>
#include <QStringList>

#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>

#include "tagtreemodel.hpp"

#include <map>


namespace _f {
    constexpr static const compsky::asciify::flag::Escape esc;
}

namespace _mysql {
	extern MYSQL* obj;
}

extern std::map<uint64_t, QString> tag_id2name;
extern QCompleter* tagcompleter;
extern QStringList tagslist;


void PrimaryItem::delete_self(){
	QStandardItem* const prnt = QStandardItem::parent();
    
    const QString qs = (prnt) ? prnt->text() : "0";
    
	constexpr static const char* const sql__delete_from = "DELETE FROM tag2parent WHERE parent_id=";
	constexpr static const char* const sql__and_tagidq  = " AND tag_id=";
	constexpr static const size_t foo = std::char_traits<const char>::length(sql__delete_from);
	static char buf[std::char_traits<char>::length(sql__delete_from) + 19 + std::char_traits<char>::length(sql__and_tagidq) + 19];
    compsky::mysql::exec(_mysql::obj, buf, sql__delete_from,  qs,  sql__and_tagidq, this->text());
    if (prnt)
        this->model()->removeRow(this->row());
    else
        prnt->removeRow(this->row());
};

void PrimaryItem::add_subtag(){
	const uint64_t child = ask_for_tag("Child Tag");
	if (child == 0)
		return;
	tag2parent(child, this->tag_id);
	this->view->add_child_to(static_cast<TagTreeModel*>(this->model()), child, this->tag_id, 0, tag_id2name[child]); // TODO: Get the text of the element to the right, i.e. the name of the tag
};

void PrimaryItem::add_parent(){
	const uint64_t parent = ask_for_tag("Parent Tag");
	if (parent == 0)
		return;
	tag2parent(this->tag_id, parent);
	this->view->add_child_to(static_cast<TagTreeModel*>(this->model()), this->tag_id, parent, 123, tag_id2name[this->tag_id]);
	// TODO: Get the text of the elements to the right, i.e. the name of the tag, and its count
}

void NameItem::setData(const QVariant& value,  const int role){
	if (this->data().toString().isNull())
        return;
    
    const QString neue = value.toString();
    if (neue.isEmpty())
        return;
    
    TagTreeModel* mdl = (TagTreeModel*)this->model();
    if (!mdl)
        // Probably during construction of NameItem during construction of TagTreeModel
        return;
    
    if (neue == mdl->tag2name[this->tag_id])
        return;
	
	tagslist[tagslist.indexOf(tag_id2name[this->tag_id])] = neue;
	tag_id2name[this->tag_id] = neue;
	delete tagcompleter;
	tagcompleter = new QCompleter(tagslist);
    
	static char buf[21 + 2*128 + 11 + 19]; // Though tag name should not be longer than 128 characters
    compsky::mysql::exec(_mysql::obj, buf, "UPDATE tag SET name=\"",  _f::esc,  '"',  neue,  "\" WHERE id=",  this->tag_id);
    
    QStandardItem::setData(value, role);
};
