/*
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
*/
#pragma once


#include <string>

#define DIR2TAG_SUBQUERY \
	"(" \
		"SELECT d2pt.id AS dir, d2t.tag " \
		"FROM dir2parent_tree d2pt " \
		"JOIN dir2tag d2t ON d2t.dir=d2pt.parent " \
		"UNION " \
		"SELECT d.id, D2t.tag " \
		"FROM dir d " \
		"JOIN device D ON D.id=d.device " \
		"JOIN device2tag D2t ON D2t.device=D.id " \
	")"

#define JOIN_TAG_BLACKLIST \
	"FROM user2blacklist_tag u2ht " \
	"JOIN tag2parent_tree t2pt ON t2pt.parent=u2ht.tag "

#define USER_DISALLOWED_FILES_INNER_PRE \
		"SELECT f2t.file AS id " \
		JOIN_TAG_BLACKLIST \
		"JOIN(" \
			"SELECT file, tag " \
			"FROM file2tag " \
			"UNION " \
			"SELECT f.id, d2t.tag " \
			"FROM file f " \
			"JOIN" DIR2TAG_SUBQUERY "d2t ON d2t.dir=f.dir" \
		")f2t ON f2t.tag=t2pt.id " \
		"WHERE u2ht.user="
#define USER_DISALLOWED_FILES(user_id) \
	"(" \
		USER_DISALLOWED_FILES_INNER_PRE, user_id, \
	")"
#define NOT_DISALLOWED_FILE(file, user_id) \
	"NOT EXISTS(" USER_DISALLOWED_FILES_INNER_PRE, user_id, " AND f2t.file=", file ")"

#define USER_DISALLOWED_ERAS_INNER_PRE \
		"SELECT e2t.era AS id " \
		JOIN_TAG_BLACKLIST \
		"JOIN(" \
			"SELECT e.id AS era, f2t.tag " \
			"FROM era e " \
			"JOIN file2tag f2t ON f2t.file=e.file " \
			"UNION " \
			"SELECT era, tag " \
			"FROM era2tag " \
			"UNION " \
			"SELECT f.id, d2t.tag " \
			"FROM file f " \
			"JOIN" DIR2TAG_SUBQUERY "d2t ON d2t.dir=f.dir " \
		")e2t ON e2t.tag=t2pt.id " \
		"WHERE u2ht.user="
#define USER_DISALLOWED_ERAS(user_id) \
	"(" \
		USER_DISALLOWED_ERAS_INNER_PRE, user_id, \
	")"
#define NOT_DISALLOWED_ERA(era_id, user_id) \
	"NOT EXISTS(" USER_DISALLOWED_ERAS_INNER_PRE, user_id, " AND id=" era_id ")"

#define USER_DISALLOWED_TAGS_INNER_PRE \
		"SELECT t2pt.id " \
		JOIN_TAG_BLACKLIST \
		"WHERE u2ht.user="
#define USER_DISALLOWED_TAGS(user_id) \
	"(" \
		USER_DISALLOWED_TAGS_INNER_PRE, user_id, \
	")"
#define NOT_DISALLOWED_TAG(tag, user_id) \
	"NOT EXISTS(" USER_DISALLOWED_TAGS_INNER_PRE, user_id, " AND t2pt.id=", tag ")"
#define USER_DISALLOWED_TAGS__COMPILE_TIME(user_id) \
	"(" \
		USER_DISALLOWED_TAGS_INNER_PRE user_id \
	")"
#define USER_DISALLOWED_DEVICES_INNER_PRE \
		"SELECT D2t.device AS id " \
		JOIN_TAG_BLACKLIST \
		"JOIN device2tag D2t ON D2t.tag=t2pt.id " \
		"WHERE u2ht.user="
#define USER_DISALLOWED_DEVICES(user_id) \
	"(" \
		USER_DISALLOWED_DEVICES_INNER_PRE, user_id, \
	")"
#define USER_DISALLOWED_DEVICES__COMPILE_TIME(user_id) \
	"(" \
		USER_DISALLOWED_DEVICES_INNER_PRE user_id \
	")"
#define USER_DISALLOWED_DIRS_INNER_PRE \
		"SELECT d2t.dir AS id " \
		JOIN_TAG_BLACKLIST \
		"JOIN" DIR2TAG_SUBQUERY "d2t ON d2t.tag=t2pt.id " \
		"WHERE u2ht.user="
#define USER_DISALLOWED_DIRS(user_id) \
	"(" \
		USER_DISALLOWED_DIRS_INNER_PRE, user_id, \
	")"
#define NOT_DISALLOWED_DIR(dir_id, user_id) \
	"NOT EXISTS(" USER_DISALLOWED_DIRS_INNER_PRE, user_id, " AND id=" dir_id ")"

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
