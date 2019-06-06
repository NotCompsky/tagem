#include "mainwindow.h"

#include "mymysql_res1.hpp" // for res1::*


TagChildTreeView::TagChildTreeView(QWidget* parent) : TagTreeView(true, parent) {
    this->setSelectionBehavior(this->SelectRows);
    this->setSelectionMode(this->SingleSelection);
    this->setDragDropMode(this->InternalMove);
    this->setDragDropOverwriteMode(false);
    
    res1::query("SELECT parent_id, B.id, B.c, B.name FROM tag2parent t2p LEFT JOIN (SELECT id, name, COUNT(A.tag_id) as c FROM tag LEFT JOIN(SELECT tag_id FROM file2tag) A ON A.tag_id = id GROUP BY id, name) B ON B.id = t2p.tag_id ORDER BY t2p.parent_id ASC");
    this->place_tags(0);
    res1::free_result();
};


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
    TagTreeModel* mdl = (TagTreeModel*)this->model();
    mdl->tag2entry.clear(); // Not unnecessary
    mdl->tag2parent.clear();
    this->init_headers();
    
    for (auto i = 0;  i < mdl->rowCount();  ++i)
        mdl->removeRow(0);
    
    auto indx = selected.indexes()[0]; // TODO: Allow display of multiple heirarchies (drag to select multiple)
    
    QString a = indx.sibling(indx.row(), 0).data().toString();
    QByteArray b = a.toLocal8Bit();
    const char* tag_id_str = b.data();
    res1::exec("CALL ancestor_tags_id_rooted_from_id(\"tmp_tag_parents\", ", tag_id_str, ')');
    res1::query_buffer("SELECT node, parent, -1, name FROM tmp_tag_parents JOIN tag ON id=parent WHERE node");
    
    this->place_tags(ascii_to_uint64(tag_id_str));
};
