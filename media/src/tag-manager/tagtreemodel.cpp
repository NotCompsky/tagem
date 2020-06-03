#include "tagtreemodel.hpp"
#include "../add_new_tag.hpp"

#include <QByteArray>
#include <QDebug> // TMP
#include <QStandardItem>
#include <QStringList>

#include <compsky/mysql/query.hpp>

#include "../completer/completer.hpp"


namespace _mysql {
	extern MYSQL* obj;
}
extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;

extern Completer tagcompleter;
extern QStringList tagslist;


TagTreeModel::TagTreeModel(int a,  int b,  QObject* parent)
// Inlined to avoid multiple definition error
:
    QStandardItemModel(a, b, parent)
{
    compsky::mysql::query_buffer(_mysql::obj, RES1, "SELECT id, name FROM tag");
    {
    uint64_t id;
    char* name;
    while (compsky::mysql::assign_next_row(RES1, &ROW1, &id, &name)){
        const QString s = name;
        this->tag2name[id] = s;
        tagslist << s;
    }
    }
	tagcompleter.reset(tagslist);
}


bool TagTreeModel::dropMimeData(const QMimeData* data,  Qt::DropAction action,  int row,  int column,  const QModelIndex& dst_parent){
    if (column == 0)
        return false;
    
	const QByteArray ba = data->data("application/x-qabstractitemmodeldatalist");
    
	const char* s = ba.data();
    
    size_t i = 0;
    
    while(s[i] < '0'  ||  s[i] > '9')
        ++i;
    
	uint64_t tag_id = s[i] - '0';
    while(i < ba.size()){
        s += 2; // Skip null byte
        if (s[i] == 0)
            break;
        tag_id *= 10;
        tag_id += s[i] - '0';
    }
    
    while((s[i] < '0'  ||  s[i] > '9')  &&  i < ba.size())
        ++i;
    
    /*
    direct_occurances = s[i] - '0';
    while(i < ba.size()){
        s += 2; // Skip null byte
        if (s[i] == 0)
            break;
        direct_occurances *= 10;
        direct_occurances += s[i] - '0';
    */
    
    const uint64_t current_parent_id = this->tag2parent[tag_id];
    
    uint64_t parent_tag_id = 0;
    
    qDebug() << dst_parent.data().toString();
    
	const QString qs = dst_parent.data().toString();
    const QByteArray ba2 = qs.toLocal8Bit();
    s = ba2.data();
    while(*s != 0){
		if(*s < '0'  ||  *s > '9')
			throw std::runtime_error("ONLY DROP MIMEDATA ON FIRST ROW (where text is the tag ID)");
        parent_tag_id *= 10;
        parent_tag_id += *s - '0';
        ++s;
    }
    
    if (parent_tag_id == current_parent_id)
        return false;
    
    if (!QStandardItemModel::dropMimeData(data, action, row, 0, dst_parent))
        return false;
    
	if (tag_id > 9999){
		throw std::runtime_error("tag_id has strangely high value");
	}
	if (parent_tag_id > 9999){
		throw std::runtime_error("parent_tag_id has strangely high value");
	}
	
	tag2parent_add(tag_id, parent_tag_id); // Register new parenthood
	tag2parent_rm(tag_id, current_parent_id);
    
    this->tag2parent[tag_id] = parent_tag_id;
    
    return true;
};
    
bool TagTreeModel::removeRows(int row,  int count,  QModelIndex parent){
	const QString tag_id = this->index(row, 0, parent).data().toString();
	const QString parent_id = parent.data().toString();
	
    if (!QStandardItemModel::removeRows(row, count, parent))
        return false;
    
	qDebug() << "DELETE FROM tag2parent WHERE parent_id=", parent_id, " AND tag_id=", tag_id;
};
