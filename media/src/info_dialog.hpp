#ifndef TAGEM_MEDIA_INFO_DIALOG_HPP
#define TAGEM_MEDIA_INFO_DIALOG_HPP


#include <QDialog>


class InfoDialog : public QDialog {
  public:
	InfoDialog(const uint64_t file_id,  QWidget* parent = nullptr);
	uint64_t file_id;
  private:
	void unlink_tag();
};


#endif
