#ifndef __TAGTREEMODEL__
#define __TAGTREEMODEL__

#include <QCompleter>

#include "unpackaged_utils.hpp" // for ascii_to_uint64

// Assumes res1 is already defined through mymysql_results.hpp import


namespace res1 {
    extern void free_result();
    extern void query_buffer(const char*);
    template<typename... Args>bool assign_next_result(Args... args);
}


class TagTreeModel : public QStandardItemModel {
    Q_OBJECT
 public:
    TagTreeModel(int a,  int b,  QWidget* parent);
    
    QStringList tagslist;
    QCompleter* tagcompleter;
    
    std::map<uint64_t, QString> tag2name;
    std::map<uint64_t, QStandardItem*> tag2entry;
    std::map<uint64_t, uint64_t> tag2parent;
    std::map<uint64_t, uint64_t> tag2directoccurances;
    std::map<uint64_t, uint64_t> tag2occurances;
    
    bool dropMimeData(const QMimeData* data,  Qt::DropAction action,  int row,  int column,  const QModelIndex& dst_parent) override {
        if (column == 0)
            return false;
        
        uint64_t tag_id;
        //uint64_t direct_occurances;
        
        QByteArray ba = data->data("application/x-qabstractitemmodeldatalist");
        
        char* s = ba.data();
        
        size_t i = 0;
        
        while(s[i] < '0'  ||  s[i] > '9')
            ++i;
        
        tag_id = s[i] - '0';
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
        
        QString qs = dst_parent.data().toString();
        ba = qs.toLocal8Bit();
        s = ba.data();
        while(*s != 0){
            parent_tag_id *= 10;
            parent_tag_id += *s - '0';
            ++s;
        }
        
        if (parent_tag_id == current_parent_id)
            return false;
        
        if (!QStandardItemModel::dropMimeData(data, action, row, 0, dst_parent))
            return false;
        
        res1::exec("INSERT IGNORE INTO tag2parent (parent_id, tag_id) VALUES (",  parent_tag_id,  ',',  tag_id,  ')');
        res1::exec("DELETE FROM tag2parent WHERE parent_id=",  current_parent_id,  " AND tag_id=",  tag_id);
        
        qDebug() << "INSERT IGNORE INTO tag2parent (parent_id, tag_id) VALUES (" << parent_tag_id << "," << tag_id << ")";
        qDebug() << "DELETE FROM tag2parent WHERE parent_id=" << current_parent_id << " AND tag_id=" << tag_id;
        
        this->tag2parent[tag_id] = parent_tag_id;
        
        return true;
    };
    
    bool removeRows(int row,  int count,  QModelIndex parent){
        QString a = this->index(row, 0, parent).data().toString();
        QByteArray b = a.toLocal8Bit();
        const char* tag_id_str = b.data();
        
        if (!QStandardItemModel::removeRows(row, count, parent))
            return false;
        
        char* parent_tag_id_str;
        if (parent_tag_id_str){
            a = parent.data().toString();
            b = a.toLocal8Bit();
            parent_tag_id_str = b.data();
        } else parent_tag_id_str = "0";
        
        qDebug() << "DELETE FROM tag2parent WHERE parent_id=", parent_tag_id_str, " AND tag_id=", tag_id_str;
    };
};

inline TagTreeModel::TagTreeModel(int a,  int b,  QWidget* parent)
// Inlined to avoid multiple definition error
:
    QStandardItemModel(a, b, parent)
{
    res1::query_buffer("SELECT id, name FROM tag");
    {
    uint64_t id;
    char* name;
    while (res1::assign_next_result(&id, &name)){
        const QString s = name;
        this->tag2name[id] = s;
        this->tagslist << s;
    }
    res1::free_result();
    }
    this->tagcompleter = new QCompleter(this->tagslist);
    
    connect(this, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(item_changed(QStandardItem*)));
}

#endif
