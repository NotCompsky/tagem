#include "tagtreeview.hpp"

#include <QDebug> // TMP
#include <QHeaderView>
#include <QList>
#include <QStandardItem>
#include <QString>
#include <QTreeView>

#include <compsky/mysql/query.hpp>


namespace _mysql {
	extern MYSQL* obj;
}
extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;



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
		this->model()->setHeaderData(3, Qt::Horizontal, "+Parent");
		this->model()->setHeaderData(4, Qt::Horizontal, "+Child");
		this->model()->setHeaderData(5, Qt::Horizontal, "Unchild");
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
    
	{
		queue.reserve(compsky::mysql::n_results<uint64_t>(RES1));
		uint64_t _parent, _tag, _count;
		char* _name;
		while (compsky::mysql::assign_next_row__no_free(RES1, &ROW1, &_parent, &_tag, &_count, &_name)){
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
		this->add_child_to(mdl, tag, parent, count, name);
    }
	
	while (compsky::mysql::assign_next_row(RES1, &ROW1));
	// Free results
};

void TagTreeView::dragMoveEvent(QDragMoveEvent* e){
    // Inlined to avoid multiple definition error
    TagTreeModel* mdl = (TagTreeModel*)this->model();
    QTreeView::dragMoveEvent(e);
};
