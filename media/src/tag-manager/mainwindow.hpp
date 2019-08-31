#ifndef __MAINWINDOW__
#define __MAINWINDOW__

#include "tagchildtreeview.hpp"
#include "tagtreemodel.hpp"
#include <QDialog>


class TagManager : public QDialog {
 private:
	TagChildTreeView* tag_child_tree_view;
 public:
	TagManager(QWidget* parent = 0);
	
	template<typename... Args>
	void add_child_to(Args... args){
		tag_child_tree_view->add_child_to((TagTreeModel*)tag_child_tree_view->model(), args...);
	}
};

#endif
