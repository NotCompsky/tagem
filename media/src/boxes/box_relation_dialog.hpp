#ifndef TAGEM_MEDIA_INSTANCE_RELATION_DIALOG_HPP
#define TAGEM_MEDIA_INSTANCE_RELATION_DIALOG_HPP

#include <QDialog>


class MainWindow;


class BoxRelationDialog : public QDialog {
  public:
	BoxRelationDialog(const uint64_t _id,  MainWindow* const _win,  QWidget* parent = nullptr);
  private:
	void toggle_display_relations(); // SLOT
	void add_tag();
	MainWindow* const win;
	const uint64_t id;
};


#endif
