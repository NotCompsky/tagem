#include "mainwindow.hpp"

#include <QVBoxLayout>

#include "mymysql_results.hpp" // for mymysql::*


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Inlined to avoid multiple definition error
    this->setWindowTitle("MyTag Tag Manager");
    QWidget* w = new QWidget();
    QVBoxLayout* vl = new QVBoxLayout();
    QHBoxLayout* hl = new QHBoxLayout();
    this->commit_btn = new QPushButton("Commit (unimplemented)");
    TagChildTreeView* tag_child_tree_view = new TagChildTreeView(this);
    hl->addWidget(tag_child_tree_view);
    TagParentTreeView* tag_parent_tree_view = new TagParentTreeView(tag_child_tree_view, this);
    hl->addWidget(tag_parent_tree_view);
    vl->addLayout(hl);
    vl->addWidget(this->commit_btn);
    w->setLayout(vl);
    this->setCentralWidget(w);
    this->show();
    
    this->add_new_tag();
}

void MainWindow::add_new_tag(){
    TagDialog* tagdialog = new TagDialog("TagDialog", "TagStr");
    tagdialog->name_edit->setCompleter(this->tagcompleter);
    
    if (tagdialog->exec() != QDialog::Accepted)
        return;
    
    QString tagstr = tagdialog->name_edit->text();
    
    if (tagstr.isEmpty())
        return;
    
    qDebug() << "added new tag: " << tagstr;
};


TagChildTreeView::TagChildTreeView(QWidget* parent) : TagTreeView(true, parent) {
    this->setSelectionBehavior(this->SelectRows);
    this->setSelectionMode(this->SingleSelection);
    this->setDragDropMode(this->InternalMove);
    this->setDragDropOverwriteMode(false);
    
    mymysql::query_buffer(&mymysql::RES, "SELECT parent_id, B.id, B.c, B.name FROM tag2parent t2p LEFT JOIN (SELECT id, name, COUNT(A.tag_id) as c FROM tag LEFT JOIN(SELECT tag_id FROM file2tag) A ON A.tag_id = id GROUP BY id, name) B ON B.id = t2p.tag_id ORDER BY t2p.parent_id ASC");
    this->place_tags(0);
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
    mymysql::exec("CALL ancestor_tags_id_rooted_from_id(\"tmp_tag_parents\", ", tag_id_str, ')');
    mymysql::query_buffer(&mymysql::RES, "SELECT node, parent, -1, name FROM tmp_tag_parents JOIN tag ON id=parent WHERE node");
    
    this->place_tags(ascii_to_uint64(tag_id_str));
};
