#include "tagtreeview.hpp"
#include "primaryitem.hpp"

#include <QDebug> // TMP
#include <QHeaderView>
#include <QList>
#include <QStandardItem>
#include <QString>
#include <QToolButton>
#include <QTreeView>

#include "tagtreemodel.hpp"

#include <compsky/mysql/query.hpp>


namespace _mysql {
	extern MYSQL* obj;
	extern MYSQL_RES* res;
	extern MYSQL_ROW row;
}



TagTreeView::TagTreeView(const bool _editable,  QWidget* parent)
// Inlined to avoid multiple definition error
:
    editable(_editable),
    QTreeView(parent)
{
    this->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    
    if (this->editable){
        TagTreeModel* mdl = new TagTreeModel(0, 5, parent);
        this->setModel(mdl);
    } else {
        TagTreeModel* mdl = new TagTreeModel(0, 3, parent);
        this->setModel(mdl);
    }
    
    this->init_headers();
};
    
void TagTreeView::init_headers(){
    // Inlined to avoid multiple definition error
    this->model()->setHeaderData(0, Qt::Horizontal, "ID");
    this->model()->setHeaderData(1, Qt::Horizontal, "Name");
    this->model()->setHeaderData(2, Qt::Horizontal, "Occurances");
    if (this->editable){
		this->model()->setHeaderData(3, Qt::Horizontal, "+Child");
		this->model()->setHeaderData(4, Qt::Horizontal, "Unchild");
    }
};

void TagTreeView::place_tags(const uint64_t root){
    // Inlined to avoid multiple definition error
    TagTreeModel* mdl = (TagTreeModel*)this->model();
    
    mdl->tag2entry.clear();
    mdl->tag2entry[root] = mdl->invisibleRootItem();
	
	struct AvadaKevadra {
		uint64_t tag;
		const uint64_t parent;
		const char* const name;
		const uint64_t count;
		
		AvadaKevadra(const uint64_t _tag,  const uint64_t _parent,  const char* const _name,  const uint64_t _count)
		: tag(_tag)
		, parent(_parent)
		, name(_name)
		, count(_count)
		{}
	};
    
    std::vector<AvadaKevadra> queue;
    queue.reserve(4096);
    
	{
		uint64_t _parent, _tag, _count;
		char* _name;
		while (compsky::mysql::assign_next_row__no_free(_mysql::res, &_mysql::row, &_parent, &_tag, &_count, &_name)){
			queue.emplace_back(_tag, _parent, _name, _count);
		}
	}
	
	size_t n_non_null_kevadras_skipped_prev = 0;
	while(true){
        /*if (parent == tag){
            qDebug() << "tag_id == parent_id\n  for tag, parent, count, name\n  " << +tag << +parent << +count << name; // (unfortunately, MySQL does not support checks
            exit(555);
        }*/
		
		size_t n_non_null_kevadras_skipped = 0;
		uint64_t tag = 0;
		uint64_t parent;
		uint64_t count;
		const char* name;
		for (size_t i = 0;  i < queue.size();  ++i){
			const AvadaKevadra kevadra = queue[i];
			tag = kevadra.tag;
			parent = kevadra.parent;
			name = kevadra.name;
			count = kevadra.count;
			if (kevadra.tag != 0){
				qDebug() << "Good:" << tag << parent << name << count;
				if (mdl->tag2entry[queue[i].parent] == nullptr){
					++n_non_null_kevadras_skipped;
					continue;
				}
				queue[i].tag = 0; // Tells us that the tag has been processed
				break;
			}
		}
		if (tag == 0){
			if (n_non_null_kevadras_skipped == 0)
				// No non-null elements were found
				break;
			else {
				if (n_non_null_kevadras_skipped == n_non_null_kevadras_skipped_prev){
					qDebug() << "Some elements failed to be pasted";
					break;
				}
				qDebug() << n_non_null_kevadras_skipped_prev << n_non_null_kevadras_skipped;
				n_non_null_kevadras_skipped_prev = n_non_null_kevadras_skipped;
				continue;
			}
		}
		
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
		
		QStandardItem* entry__addchild;
		QStandardItem* entry__delete;
		if (this->editable){
			entry__addchild = new QStandardItem();
			entry__addchild->setEditable(false);
			entry__addchild->setDropEnabled(false);
			ls << entry__addchild;
			
			entry__delete = new QStandardItem();
			entry__delete->setEditable(false);
			entry__delete->setDropEnabled(false);
			ls << entry__delete;
		}
		mdl->tag2entry[parent]->appendRow(ls);
		if (this->editable){
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
		
		mdl->tag2entry[tag] = entry__id;
        mdl->tag2parent[tag] = parent;
        mdl->tag2name[tag] = name;
        mdl->tag2directoccurances[tag] = mdl->tag2occurances[tag] = count;
        mdl->tag2occurances[parent] += count;
    }
	
	while (compsky::mysql::assign_next_row(_mysql::res, &_mysql::row));
	// Free results
};

void TagTreeView::dragMoveEvent(QDragMoveEvent* e){
    // Inlined to avoid multiple definition error
    TagTreeModel* mdl = (TagTreeModel*)this->model();
    QTreeView::dragMoveEvent(e);
};
