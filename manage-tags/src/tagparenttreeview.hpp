#ifndef __TAGPARENTTREEVIEW__
#define __TAGPARENTTREEVIEW__


#include <QItemSelection>

#include "tagtreeview.hpp"


class TagChildTreeView;


class TagParentTreeView : public TagTreeView {
    Q_OBJECT
 public:
    TagParentTreeView(TagChildTreeView* tag_child_tree_view,  QWidget* parent);
    TagChildTreeView* tag_child_tree_view;
 public Q_SLOTS:
    void set_root(const QItemSelection& selected,  const QItemSelection& deselected);
};


#endif
