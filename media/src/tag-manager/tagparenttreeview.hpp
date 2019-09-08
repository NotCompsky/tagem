#ifndef __TAGPARENTTREEVIEW__
#define __TAGPARENTTREEVIEW__


#include <QItemSelection>

#include "tagtreeview.hpp"


class TagChildTreeView;


class TagParentTreeView : public TagTreeView {
private:
	void set_root_from(const uint64_t tag_id);
 public:
    TagParentTreeView(TagChildTreeView* tag_child_tree_view,  QWidget* parent);
    TagChildTreeView* tag_child_tree_view;
	void set_root(const QItemSelection& selected,  const QItemSelection& deselected); // SLOT
	void jump_to(); // SLOT
};


#endif
