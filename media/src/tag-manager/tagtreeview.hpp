#ifndef __TAGTREEVIEW__
#define __TAGTREEVIEW__

#include "tagtreemodel.hpp"
#include "primaryitem.hpp"
#include <QToolButton>
#include <QTreeView>


namespace _detail {
	inline const char* get_const_char_p(const char* const s){
		return s;
	}
	inline const char* get_const_char_p(const QString& qs){
		const QByteArray ba = qs.toLocal8Bit();
		const char* s = ba.data();
		return s;
	}
}


class TagTreeView : public QTreeView {
 private:
	const bool editable;
	void dragMoveEvent(QDragMoveEvent* e);
 public:
	TagTreeView(const bool _editable,  QWidget* parent);
	
    void init_headers();
	
	void place_tags(const uint64_t root);
	
	template<typename T>
	void add_child_to(
		TagTreeModel* const mdl,
		const uint64_t tag,
		const uint64_t parent,
		const uint64_t count,
		T name
	){
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
		
		QStandardItem* entry__addparent;
		QStandardItem* entry__addchild;
		QStandardItem* entry__delete;
		if (this->editable){
			entry__addparent = new QStandardItem();
			entry__addparent->setEditable(false);
			entry__addparent->setDropEnabled(false);
			ls << entry__addparent;
			
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
			QToolButton* addparent_btn = new QToolButton();
			addparent_btn->setText("^");
			addparent_btn->setMaximumSize(addparent_btn->sizeHint());
			this->setIndexWidget(entry__addparent->index(), addparent_btn);
			connect(addparent_btn, &QToolButton::clicked, entry__id, &PrimaryItem::add_parent);
			
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
		mdl->tag2name[tag] = _detail::get_const_char_p(name);
		mdl->tag2directoccurances[tag] = mdl->tag2occurances[tag] = count;
		mdl->tag2occurances[parent] += count;
	}
};

#endif
