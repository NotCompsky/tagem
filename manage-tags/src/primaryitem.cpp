#include "primaryitem.hpp"

#include <QStandardItem>

#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>


#include "tagtreemodel.hpp"


namespace _f {
    constexpr static const compsky::asciify::flag::Escape esc;
}

namespace _mysql {
	extern MYSQL* obj;
	extern MYSQL_ROW row;
}

extern char BUF[];


void PrimaryItem::delete_self(){
    QStandardItem* prnt = QStandardItem::parent();
    
    const QString qs = (prnt) ? prnt->text() : "0";
    
    compsky::mysql::exec(_mysql::obj, BUF, "DELETE FROM tag2parent WHERE parent_id=",  qs,  " AND tag_id=", this->text());
    if (prnt)
        this->model()->removeRow(this->row());
    else
        prnt->removeRow(this->row());
};

void PrimaryItem::add_subtag(){
};

void PrimaryItem::add_parent(){
}

void NameItem::setData(const QVariant& value,  int role){
    QString a = this->data().toString();
    QByteArray b = a.toLocal8Bit();
    
    const char* s = b.data(); // Equivalent to siblingAtColumn, but that is introduced in Qt 5.11
    
    if (!s)
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
    
    compsky::mysql::exec(_mysql::obj, BUF, "UPDATE tag SET name=\"",  _f::esc,  '"',  neue,  "\" WHERE id=",  this->tag_id);
    
    QStandardItem::setData(value, role);
};
