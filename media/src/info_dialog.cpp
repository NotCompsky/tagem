#include "info_dialog.hpp"
#include "unlink_tag_btn.hpp"

#include <compsky/mysql/query.hpp>

#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QHBoxLayout>
#include <QVBoxLayout>


namespace _mysql {
	extern MYSQL* obj;
}

extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;
extern char BUF[];


InfoDialog::InfoDialog(const uint64_t file_id,  const qint64 file_sz,  QWidget* parent)
: QDialog(parent)
, file_id(file_id)
{
	QVBoxLayout* l = new QVBoxLayout(this);
	
	l->addWidget(new QLabel(QString("ID: %1").arg(this->file_id)));
	
	QLocale locale = this->locale();
	l->addWidget(new QLabel(QString("File size: ") + locale.formattedDataSize(file_sz)));
	
	const char* _score_str;
	compsky::mysql::query(_mysql::obj, RES1, BUF, "SELECT score FROM file WHERE id=", this->file_id);
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &_score_str)){
		l->addWidget(new QLabel(QString("Score: %1").arg(_score_str)));
	}
	
	const char* _file_path;
	compsky::mysql::query(_mysql::obj, RES1, BUF, "SELECT name FROM file WHERE id=", this->file_id);
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &_file_path)){
		l->addWidget(new QLineEdit(_file_path));
	}
	
	compsky::mysql::query(_mysql::obj, RES1, BUF, "SELECT t.id, t.name FROM file2tag f2t, tag t WHERE t.id=f2t.tag_id AND f2t.file_id=", this->file_id);
	uint64_t _tagid;
	const char* _tagname;
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &_tagid, &_tagname)){
		QHBoxLayout* hbox = new QHBoxLayout;
		
		hbox->addWidget(new QLabel(_tagname));
		hbox->addWidget(new UnlinkTagBtn("DELETE FROM file2tag WHERE file_id=", this->file_id, " AND tag_id=",_tagid, this));
		
		l->addLayout(hbox);
	}
}
