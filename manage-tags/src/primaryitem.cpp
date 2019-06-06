#include "primaryitem.hpp"

#include "asciify_flags.hpp"
#include "tagtreemodel.hpp"

#include "mymysql_res1.hpp" // for res1::*


void PrimaryItem::delete_self(){
    QStandardItem* prnt = this->parent();
    
    const QString qs = (prnt) ? prnt->text() : "0";
    
    res1::exec("DELETE FROM tag2parent WHERE parent_id=",  qs,  " AND tag_id=", this->text());
    if (prnt)
        this->model()->removeRow(this->row());
    else
        prnt->removeRow(this->row());
};

void PrimaryItem::add_subtag(){
};

void NameItem::setData(const QVariant& value,  int role){
    QString a = this->data().toString();
    QByteArray b = a.toLocal8Bit();
    
    const char* s = b.data(); // Equivalent to siblingAtColumn, but that is introduced in Qt 5.11
    
    if (!s)
        return;
    
    const QString qs = value.toString();
    const QByteArray ba = qs.toLocal8Bit();
    const char* neue = ba.data();
    if (neue[0] == 0)
        return;
    
    TagTreeModel* mdl = (TagTreeModel*)this->model();
    if (!mdl)
        // Probably during construction of NameItem during construction of TagTreeModel
        return;
    
    if (qs == mdl->tag2name[this->tag_id])
        return;
    
    res1::exec("UPDATE tag SET name=\"",  compsky::asciify::flag::escape,  '"',  neue,  "\" WHERE id=",  this->tag_id);
    
    QStandardItem::setData(value, role);
};
