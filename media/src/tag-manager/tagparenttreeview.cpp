#include "tagparenttreeview.hpp"
#include "../add_new_tag.hpp"

#include <QByteArray>
#include <QString>

#include <compsky/mysql/query.hpp>

#include "tagchildtreeview.hpp"
#include "tagtreemodel.hpp"
#include "unpackaged_utils.hpp" // for ascii_to_uint64


namespace _mysql {
	extern MYSQL* obj;
}
extern MYSQL_RES* RES1;


constexpr static const char* const sql__call_ancestor_tags_str = "CALL ancestor_tags_id_rooted_from_id(\"tmp_tag_parents\", ";


TagParentTreeView::TagParentTreeView(TagChildTreeView* tag_child_tree_view,  QWidget* parent)
:
    tag_child_tree_view(tag_child_tree_view),
    TagTreeView(false, parent)
{
    connect(tag_child_tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TagParentTreeView::set_root);
    
    //connect(tag_child_tree_view.selectionModel(), QItemSelectionModel::selectionChanged(const QItemSelection&, const QItemSelection&), &TagParentTreeView::selectionChanged, this, SLOT(set_root()));
    
    this->setSelectionBehavior(this->SelectRows);
    this->setSelectionMode(this->SingleSelection);
    this->setDragDropOverwriteMode(false);
};

void TagParentTreeView::set_root(const QItemSelection& selected,  const QItemSelection& deselected){
	static char buf[std::char_traits<char>::length(sql__call_ancestor_tags_str) + 19 + 1];
    TagTreeModel* mdl = (TagTreeModel*)this->model();
    mdl->tag2entry.clear(); // Not unnecessary
    mdl->tag2parent.clear();
    this->init_headers();
    
    for (auto i = 0;  i < mdl->rowCount();  ++i)
        mdl->removeRow(0);
    
    auto indx = selected.indexes()[0]; // TODO: Allow display of multiple heirarchies (drag to select multiple)
    
	const QString a = indx.sibling(indx.row(), 0).data().toString();
	const QByteArray b = a.toLocal8Bit();
    const char* tag_id_str = b.data();
    compsky::mysql::exec(_mysql::obj,  buf,  sql__call_ancestor_tags_str, tag_id_str, ')');
    compsky::mysql::query_buffer(_mysql::obj, RES1,  "SELECT node, parent, -1, name FROM tmp_tag_parents JOIN tag ON id=parent WHERE node");
    
    this->place_tags(ascii_to_uint64(tag_id_str));
};

void TagParentTreeView::set_root_from(const uint64_t tag_id){
	static char buf[std::char_traits<char>::length(sql__call_ancestor_tags_str) + 19 + 1];
    compsky::mysql::exec(_mysql::obj,  buf,  sql__call_ancestor_tags_str, tag_id, ')');
    compsky::mysql::query_buffer(_mysql::obj, RES1,  "SELECT node, parent, -1, name FROM tmp_tag_parents JOIN tag ON id=parent WHERE node");
    
    this->place_tags(tag_id);
};

void TagParentTreeView::jump_to(){
	const uint64_t tag = ask_for_tag("");
	this->set_root_from(tag);
	this->tag_child_tree_view->scrollTo(static_cast<TagTreeModel*>(this->tag_child_tree_view->model())->tag2entry[tag]->index());
}
