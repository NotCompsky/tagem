#include <QDebug> // TMP
#include <QHeaderView>

#include "tagtreeview.hpp"
#include "tagtreemodel.hpp"

#include "mymysql_res1.hpp" // for res1::*


AvadaKevadra::AvadaKevadra(const uint64_t tag,  const uint64_t parent,  const char* name,  const uint64_t count) : tag(tag), parent(parent), name(name), count(count) {}



TagTreeView::TagTreeView(bool editable,  QWidget* parent)
// Inlined to avoid multiple definition error
:
    editable(editable),
    QTreeView(parent)
{
    this->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    
    if (editable){
        TagTreeModel* mdl = new TagTreeModel(0, 5, parent);
        this->setModel(mdl);
    } else {
        QStandardItemModel* mdl = new QStandardItemModel(0, 3, parent);
        this->setModel(mdl);
    }
    
    this->init_headers();
};
    
void TagTreeView::init_headers(){
    // Inlined to avoid multiple definition error
    this->model()->setHeaderData(0, Qt::Horizontal, "ID");
    this->model()->setHeaderData(1, Qt::Horizontal, "Name");
    this->model()->setHeaderData(2, Qt::Horizontal, "Occurances");
    if (editable){
        this->model()->setHeaderData(3, Qt::Horizontal, "Unchild");
        this->model()->setHeaderData(4, Qt::Horizontal, "+Child");
    }
};

void TagTreeView::place_tags(uint64_t root){
    // Inlined to avoid multiple definition error
    TagTreeModel* mdl = (TagTreeModel*)this->model();
    
    mdl->tag2entry.clear();
    mdl->tag2entry[root] = mdl->invisibleRootItem();
    
    std::vector<AvadaKevadra> queue;
    queue.reserve(1024);
    
    uint64_t parent, tag, count;
    char* name;
    while (res1::assign_next_result(&parent, &tag, &count, &name)){
        if (parent == tag){
            qDebug() << name << ": tag_id == parent_id == ", tag, " (unfortunately, MySQL does not support checks)";
            exit(555);
        }
        
        if (mdl->tag2entry.find(parent) == mdl->tag2entry.end()){ // TODO: Replace with std::map::contains (C++20) in a few decades
            queue.emplace_back(tag, parent, name, count);
            continue;
        }
        
        qDebug() << tag << name << parent << count;
        
        PrimaryItem* entry__id = new PrimaryItem(QString("%0").arg(tag));
        entry__id->setEditable(false);
        entry__id->setDropEnabled(true);
        
        NameItem* entry__name = new NameItem(tag, name);
        entry__name->setEditable(true);
        entry__name->setDropEnabled(false);
        
        StandardItem* entry__count = new StandardItem(QString("%0").arg(count));
        entry__count->setEditable(false);
        entry__count->setDropEnabled(false);
        
        QList<QStandardItem*> ls = {(QStandardItem*)entry__id, (QStandardItem*)entry__name, (QStandardItem*)entry__count};
        
        if (this->editable){
            QStandardItem* entry__addchild = new QStandardItem();
            entry__addchild->setEditable(false);
            entry__addchild->setDropEnabled(false);
            ls << entry__addchild;
            
            QStandardItem* entry__delete = new QStandardItem();
            entry__delete->setEditable(false);
            entry__delete->setDropEnabled(false);
            ls << entry__delete;
            
            QToolButton* addchild_btn = new QToolButton();
            addchild_btn->setText("+");
            addchild_btn->setMaximumSize(addchild_btn->sizeHint());
            this->setIndexWidget(entry__addchild->index(), addchild_btn);
            connect(addchild_btn, &QToolButton::clicked, entry__id, &PrimaryItem::add_subtag);
            
            QToolButton* delete_btn = new QToolButton();
            delete_btn->setText("X");
            delete_btn->setMaximumSize(delete_btn->sizeHint());
            this->setIndexWidget(entry__delete->index(), delete_btn);
            connect(delete_btn, &QToolButton::clicked, entry__id, &PrimaryItem::delete_self);
        }
        
        mdl->tag2entry[parent]->appendRow(ls);
        
        mdl->tag2parent[tag] = parent;
        mdl->tag2entry[tag] = entry__id;
        mdl->tag2name[tag] = name;
        mdl->tag2directoccurances[tag] = mdl->tag2occurances[tag] = count;
        mdl->tag2occurances[parent] += count;
    }
};

void TagTreeView::dragMoveEvent(QDragMoveEvent* e){
    // Inlined to avoid multiple definition error
    TagTreeModel* mdl = (TagTreeModel*)this->model();
    QTreeView::dragMoveEvent(e);
};
