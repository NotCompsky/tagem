#ifndef __TAGTREEMODEL__
#define __TAGTREEMODEL__

#include <map> // for std::map
#include <vector>

#include <QCompleter>
#include <QMimeData>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QStringList>

// Assumes res1 is already defined through mymysql_results.hpp import


class TagTreeModel : public QStandardItemModel {
 public:
    TagTreeModel(int a,  int b,  QObject* parent);
    
    QStringList tagslist;
    QCompleter* tagcompleter;
    
    std::map<uint64_t, QString> tag2name;
    std::map<uint64_t, std::vector<QStandardItem*>> tag2entries;
    std::map<uint64_t, uint64_t> tag2parent;
    std::map<uint64_t, uint64_t> tag2directoccurances;
    std::map<uint64_t, uint64_t> tag2occurances;
    
    bool dropMimeData(const QMimeData* data,  Qt::DropAction action,  int row,  int column,  const QModelIndex& dst_parent) override;
    
    bool removeRows(int row,  int count,  QModelIndex parent);
};

#endif
