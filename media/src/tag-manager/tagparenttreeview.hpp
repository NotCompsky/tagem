#ifndef __TAGPARENTTREEVIEW__
#define __TAGPARENTTREEVIEW__


#include <QItemSelection>

#include "tagtreeview.hpp"


class TagChildTreeView;


class TagParentTreeView : public TagTreeView {
 public:
    TagParentTreeView(TagChildTreeView* tag_child_tree_view,  QWidget* parent);
    TagChildTreeView* tag_child_tree_view;
	void set_root(const QItemSelection& selected,  const QItemSelection& deselected); // SLOT
};


#endif
