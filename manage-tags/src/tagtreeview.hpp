#ifndef __TAGTREEVIEW__
#define __TAGTREEVIEW__

#include <QToolButton>
#include <QTreeView>

#include "primaryitem.hpp"


struct AvadaKevadra {
    AvadaKevadra(const uint64_t tag,  const uint64_t parent,  const char* name,  const uint64_t count);
    // Explicitly defined to allow use of emplace_back
    
    const uint64_t tag;
    const uint64_t parent;
    const char* name;
    const uint64_t count;
};


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
