#include "add_new_tag.hpp"
#include "name_dialog.hpp"
#include "tag-manager/mainwindow.hpp"
#include <compsky/mysql/query.hpp>
#include <map>
#include "completer/completer.hpp"
#include <QStringList>


constexpr static const compsky::asciify::flag::Escape f_esc;

namespace _mysql {
	extern MYSQL* obj;
}

extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;
extern MYSQL_RES* RES2;
extern MYSQL_ROW ROW2;


extern Completer tagcompleter;
extern QStringList tagslist;
extern std::map<uint64_t, QString> tag_id2name;

#if (defined TAG_MANAGER || defined MAINWINDOW)
extern TagManager* tag_manager;
#endif


template<typename String>
uint64_t get_id_from_table(const char* const table_name,  const String entry_name){
	uint64_t value = 0;

	goto__returnresultsofthegetidfromtablequery:
	static char buf[1024]; // WARNING: Arbitrary size
	compsky::mysql::query(_mysql::obj,  RES1,  buf,  "SELECT id FROM ", table_name, " WHERE name=\"", f_esc, '"', entry_name, '"');

	while(compsky::mysql::assign_next_row(RES1,  &ROW1,  &value));

	if (value)
		return value;

	compsky::mysql::exec(_mysql::obj,  buf,  "INSERT INTO ", table_name, " (name) VALUES (\"", f_esc, '"', entry_name, "\")");
	goto goto__returnresultsofthegetidfromtablequery;
};

void tag2parent(const uint64_t tagid,  const uint64_t parid){
	constexpr static const char* const sql__insert =       "INSERT IGNORE INTO tag2parent (tag_id, parent_id) VALUES (";
	constexpr static const char* const sql__update_perms = "UPDATE tag t JOIN tag2parent t2p ON t2p.tag_id=t.id JOIN tag p ON p.id=t2p.parent_id SET t.permissions=t.permissions|p.permissions";
	
	static char buf[std::char_traits<char>::length(sql__update_perms)];
	
	compsky::mysql::exec(_mysql::obj,  buf,  sql__insert, tagid, ',', parid, ")");
	compsky::mysql::exec(_mysql::obj,  buf,  sql__update_perms);
	compsky::mysql::exec(_mysql::obj,  BUF,  "INSERT INTO tag2parent_tree (tag, parent, depth) SELECT ", tagid, ",parent,depth+1 FROM tag2parent_tree WHERE tag=", parid, " ON DUPLICATE KEY UPDATE tag=tag");
}

uint64_t add_new_tag(const QString& tagstr,  uint64_t tagid){
	// TODO: Do all of these in one transaction, so that incomplete tags are never added.
	
	tagslist.append(tagstr);
	tagcompleter.reset(tagslist);

	if (tagid == 0)
		tagid = get_id_from_table("tag", tagstr);

	tag_id2name[tagid] = tagstr;

	/* Get parent tag */
	QString parent_tagstr;
	QByteArray parent_tagstr_ba;
	char* parent_tagchars;
	while(true){
		NameDialog* tagdialog = new NameDialog("Parent Tag of", tagstr);
		tagdialog->name_edit->setCompleter(tagcompleter.completer);
		const int _rc = tagdialog->exec();
		parent_tagstr = tagcompleter.get_orig_str(tagdialog->name_edit->text());
		delete tagdialog;
		
		if (_rc != QDialog::Accepted  ||  parent_tagstr == tagstr)
			continue;
		if (parent_tagstr.isEmpty()){
			tag2parent(tagid, 0);
			return tagid;
		}
		parent_tagstr_ba = parent_tagstr.toLocal8Bit();
		parent_tagchars = parent_tagstr_ba.data();
		break;
	}

	/* Insert parent-child relations */
	// TODO: Replace this crude system with the option to add textboxes to name more parents
	int lastindx = 0;
	for (auto i = 0;  ;  ++i)
		if (parent_tagchars[i] == '|'  ||  parent_tagchars[i] == 0){
			char iszero = parent_tagchars[i];
			parent_tagchars[i] = 0;
			
			auto parid = get_id_from_table("tag",  parent_tagchars + lastindx);
			
			tag2parent(tagid, parid);
			
			QString parstr = QString::fromLocal8Bit(parent_tagchars + lastindx);
			if (!tagslist.contains(parstr))
				add_new_tag(parstr, parid);
			
		#if (defined TAG_MANAGER || defined MAINWINDOW)
			if (tag_manager != nullptr)
				tag_manager->add_child_to(tagid,  parid,  1,  tagstr);
		#endif
			if (iszero == 0)
				break;
			
			lastindx = i + 1;
		}

	return tagid;
}

uint64_t ask_for_tag(const QString& str){
	NameDialog* tagdialog = new NameDialog("Tag", str);
	tagdialog->name_edit->setCompleter(tagcompleter.completer);
	const auto rc = tagdialog->exec();
	const QString tagstr = tagcompleter.get_orig_str(tagdialog->name_edit->text());

	if (rc != QDialog::Accepted  ||  tagstr.isEmpty())
		return 0;
	
	return  (tagslist.contains(tagstr))  ?  get_id_from_table("tag", tagstr)  :  add_new_tag(tagstr);
}
