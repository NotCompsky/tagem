#pragma once


#include <string>


#define USER_DISALLOWED_FILES_INNER_PRE \
		"SELECT f2t.file AS id " \
		"FROM user2blacklist_tag u2ht " \
		"JOIN tag2parent_tree t2pt ON t2pt.parent=u2ht.tag " \
		"JOIN file2tag f2t ON f2t.tag=t2pt.tag " \
		"WHERE u2ht.user="
#define USER_DISALLOWED_FILES_INNER_PRE2 \
		"SELECT f.id " \
		"FROM _file f " \
		"JOIN user2blacklist_dir d ON d.dir=f.dir " \
		"WHERE d.user="
#define USER_DISALLOWED_FILES(user_id) \
	"(" \
		USER_DISALLOWED_FILES_INNER_PRE, user_id, \
		" UNION " \
		USER_DISALLOWED_FILES_INNER_PRE2, user_id, \
	")"
#define FILE_TBL_USER_PERMISSION_FILTER(user_id) \
	"AND f.id NOT IN" USER_DISALLOWED_FILES(user_id)
#define USER_DISALLOWED_TAGS_INNER_PRE \
		"SELECT t2pt.tag AS id " \
		"FROM user2blacklist_tag u2ht " \
		"JOIN tag2parent_tree t2pt ON t2pt.parent=u2ht.tag " \
		"WHERE u2ht.user="
#define USER_DISALLOWED_TAGS(user_id) \
	"(" \
		USER_DISALLOWED_TAGS_INNER_PRE, user_id, \
	")"
#define USER_DISALLOWED_TAGS__COMPILE_TIME(user_id) \
	"(" \
		USER_DISALLOWED_TAGS_INNER_PRE user_id \
	")"
#define TAG_TBL_USER_PERMISSION_FILTER(user_id) \
	"AND t.id NOT IN" USER_DISALLOWED_TAGS(user_id)
#define USER_DISALLOWED_DEVICES_INNER_PRE \
		"SELECT device AS id " \
		"FROM user2blacklist_device " \
		"WHERE user="
#define USER_DISALLOWED_DEVICES(user_id) \
	"(" \
		USER_DISALLOWED_DEVICES_INNER_PRE, user_id, \
	")"
#define USER_DISALLOWED_DEVICES__COMPILE_TIME(user_id) \
	"(" \
		USER_DISALLOWED_DEVICES_INNER_PRE user_id \
	")"
#define USER_DISALLOWED_DIRS_INNER_PRE \
		"SELECT dir AS id " \
		"FROM user2blacklist_dir " \
		"WHERE user="
#define USER_DISALLOWED_DIRS(user_id) \
	"(" \
		USER_DISALLOWED_DIRS_INNER_PRE, user_id, \
		" UNION " \
		"SELECT id FROM _dir WHERE id NOT IN(SELECT dir FROM _file WHERE id NOT IN" USER_DISALLOWED_FILES(user_id) ")" \
	")"
#define USER_DISALLOWED_DIRS__COMPILE_TIME(user_id) \
	"(" \
		USER_DISALLOWED_DIRS_INNER_PRE user_id \
	")"
#define DIR_TBL_USER_PERMISSION_FILTER(user_id) \
	"AND d.id NOT IN" USER_DISALLOWED_DIRS(user_id)


namespace sql_factory{

namespace successness {
	enum ReturnType {
		ok,
		unimplemented,
		invalid,
		malicious
	};
}

successness::ReturnType parse_into(char* itr,  const char* qry,  const std::string& connected_local_devices_str,  const unsigned user_id);


} // namespace sql_factory
