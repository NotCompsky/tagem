#include "info_dialog.hpp"
#include "unlink_tag_btn.hpp"
#include "file2.hpp"
#include "basename.hpp"

#include <compsky/mysql/query.hpp>

#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <cstdio> // for rename


namespace _mysql {
	extern MYSQL* obj;
}

constexpr static const compsky::asciify::flag::Escape f_esc;
constexpr static const compsky::asciify::flag::StrLen f_strlen;

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
	
	for (const QString& var_name : file2::names){
		const char* _x;
		compsky::mysql::query(_mysql::obj, RES1, BUF, "SELECT x FROM file2", var_name, " WHERE file=", this->file_id);
		while(compsky::mysql::assign_next_row(RES1, &ROW1, &_x)){
			l->addWidget(new QLabel(QString("%1: %2").arg(var_name).arg(_x)));
		}
	}
	
	const char* _file_path;
	compsky::mysql::query(_mysql::obj, RES1, BUF, "SELECT CONCAT(d.name, '/', f.name) FROM file f JOIN dir d ON d.id=f.dir WHERE f.id=", this->file_id);
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &_file_path)){
		QLineEdit* _file_path_line_edit = new QLineEdit(_file_path);
		memcpy(this->file_path,  _file_path,  strlen(_file_path) + 1);
		connect(_file_path_line_edit, &QLineEdit::editingFinished, this, &InfoDialog::update_file_path);
		l->addWidget(_file_path_line_edit);
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

void InfoDialog::update_file_path(){
	QLineEdit* const line_edit = static_cast<QLineEdit*>(sender());
	const QString s = line_edit->text();
	
	if (s.isEmpty()){
		line_edit->setText(this->file_path);
		return;
	}
	
	const QByteArray ba = s.toLocal8Bit();
	const char* const _file_path = ba.data();
	
	const int rc = rename(this->file_path, _file_path);
	
	if (rc != 0){
		line_edit->setText(this->file_path);
		return;
	}
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"INSERT INTO dir "
		"SELECT \"", f_esc, '"', f_strlen, pardir_length(_file_path), _file_path, "\" "
		"FROM dir "
		"WHERE NOT EXISTS ("
			"SELECT id "
			"FROM dir "
			"WHERE name=\"", f_esc, '"', f_strlen, pardir_length(_file_path), _file_path, "\""
		")"
		"LIMIT 1"
	);
	// WARNING: What happens if user tries to insert a directory which already exists in '_dir', but which they do not have permission to view (thus not appearing in the 'dir' view)?
	
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"UPDATE file "
		"SET name=\"",
			f_esc, '"', basename(_file_path),
		"\", dir=(SELECT id FROM dir WHERE name=\"", f_esc, '"', f_strlen, pardir_length(_file_path), _file_path, "\")"
		"WHERE id=", this->file_id
	);
	
	memcpy(this->file_path,  _file_path,  strlen(_file_path) + 1);
}
