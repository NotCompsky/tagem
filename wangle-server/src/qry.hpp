#pragma once


#define USER_DISALLOWED_FILES_INNER_PRE \
		"SELECT f2t.file " \
		"FROM user2blacklist_tag u2ht " \
		"JOIN tag2parent_tree t2pt ON t2pt.parent=u2ht.tag " \
		"JOIN file2tag f2t ON f2t.tag=t2pt.tag " \
		"WHERE u2ht.user="
#define USER_DISALLOWED_FILES(user_id) \
	"(" \
		USER_DISALLOWED_FILES_INNER_PRE, user_id, \
	")"
#define FILE_TBL_USER_PERMISSION_FILTER(user_id) \
	"AND f.id NOT IN" USER_DISALLOWED_FILES(user_id)
#define USER_DISALLOWED_TAGS_INNER_PRE \
		"SELECT t2pt.tag " \
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
		"SELECT device " \
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
		"SELECT dir " \
		"FROM user2blacklist_dir " \
		"WHERE user="
#define USER_DISALLOWED_DIRS(user_id) \
	"(" \
		USER_DISALLOWED_DIRS_INNER_PRE, user_id, \
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

__attribute__((visibility("default")))
extern "C"
successness::ReturnType parse_into(char* itr,  const char* qry,  const unsigned user_id);


} // namespace sql_factory
