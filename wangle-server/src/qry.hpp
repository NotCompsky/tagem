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


#define JOIN_TAG_BLACKLIST \
	"FROM user2blacklist_tag u2ht " \
	"JOIN tag2parent_tree _t2pt ON _t2pt.parent=u2ht.tag "


#define NOT_DISALLOWED_TAG__BASE(...) \
	"NOT EXISTS(" \
		"SELECT _t2pt.id " \
		JOIN_TAG_BLACKLIST \
		"WHERE u2ht.user=" __VA_ARGS__ \
	")"
#define NOT_DISALLOWED_TAG(tag, user_id) \
	NOT_DISALLOWED_TAG__BASE("", user_id, " AND _t2pt.id=", tag)
#define NOT_DISALLOWED_TAG__COMPILE_TIME(tag, user_id) \
	NOT_DISALLOWED_TAG__BASE(user_id " AND _t2pt.id=" tag)
#define USER_DISALLOWED_DEVICES_INNER_PRE \
		"SELECT D2t.device AS id " \
		JOIN_TAG_BLACKLIST \
		"JOIN device2tag D2t ON D2t.tag=_t2pt.id " \
		"WHERE u2ht.user="
#define NOT_DISALLOWED_DEVICE(device, user_id) \
	"NOT EXISTS(" \
		USER_DISALLOWED_DEVICES_INNER_PRE, user_id, \
		" AND D2t.device=" device \
	")"
#define NOT_DISALLOWED_DEVICE__COMPILE_TIME(device, user_id) \
	"NOT EXISTS(" \
		USER_DISALLOWED_DEVICES_INNER_PRE user_id \
		" AND D2t.device=" device \
	")"
#define NOT_DISALLOWED_DIR(dir_id, device_id, user_id) \
	"NOT EXISTS(" \
		"SELECT d2t.dir AS id " \
		JOIN_TAG_BLACKLIST \
		"JOIN dir2tag d2t ON d2t.tag=_t2pt.id " \
		"WHERE u2ht.user=", user_id, " AND id=" dir_id \
	")" \
	"AND " NOT_DISALLOWED_DEVICE(device_id, user_id)
#define NOT_DISALLOWED_FILE(file, dir, device, user_id) \
	"NOT EXISTS(" \
		"SELECT f2t.file AS id " \
		JOIN_TAG_BLACKLIST \
		"JOIN file2tag f2t ON f2t.tag=_t2pt.id " \
		"WHERE u2ht.user=", user_id, " " \
		"AND f2t.file=", file \
	")" \
	"AND " NOT_DISALLOWED_DIR(dir, device, user_id)
#define NOT_DISALLOWED_ERA(era, file, dir, device, user_id) \
	"NOT EXISTS(" \
		"SELECT e2t.era AS id " \
		JOIN_TAG_BLACKLIST \
		"JOIN era2tag e2t ON e2t.tag=_t2pt.id " \
		"WHERE u2ht.user=", user_id, " AND id=" era \
	")" \
	"AND " NOT_DISALLOWED_FILE(file, dir, device, user_id)

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
