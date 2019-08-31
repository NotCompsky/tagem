#ifndef __TAGTREEVIEW__
#define __TAGTREEVIEW__

#include <QTreeView>


class TagTreeView : public QTreeView {
 private:
	const bool editable;
	void dragMoveEvent(QDragMoveEvent* e);
 public:
	TagTreeView(const bool _editable,  QWidget* parent);
    void init_headers();
    void place_tags(uint64_t root);
};

#endif
