#ifndef __TAGTREEVIEW__
#define __TAGTREEVIEW__

#include <QToolButton>
#include <QTreeView>

#include "primaryitem.hpp"


class TagTreeView : public QTreeView {
    Q_OBJECT
 public:
    TagTreeView(bool editable,  QWidget* parent);
    
    void init_headers();
    void place_tags(uint64_t root);
    void dragMoveEvent(QDragMoveEvent* e);
    
    bool editable;
};

#endif
