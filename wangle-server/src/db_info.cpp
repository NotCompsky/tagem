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

#include "db_info.hpp"
#include "log.hpp"
#include <compsky/mysql/query.hpp>
#include <compsky/utils/streq.hpp>
#include <compsky/mysql/except.hpp>


void DatabaseInfo::close(){
	mysql_close(mysql_obj);
	compsky::mysql::wipe_auth(buf, buf_sz);
}


void DatabaseInfo::test_is_accessible_from_master_connection(MYSQL* const master_connection,  char* buf){
	try {
		MYSQL_RES* res;
		compsky::mysql::query(master_connection, res, buf, "SELECT 1 FROM ", this->name(), ".post LIMIT 1", '\0');
		mysql_free_result(res);
		this->bools[is_accessible_from_master_connection] = true;
	} catch(const compsky::mysql::except::SQLExec&){
		this->logs("Warning: Unable to access database");
	}
}


void DatabaseInfo::attempt_to_access_tbl(const char* const tbl_name) const {
	MYSQL_RES* res;
	MYSQL_ROW  row;
	static char buf[100];
	try {
		compsky::mysql::query(this->mysql_obj, res, buf, "SELECT * FROM ", tbl_name, " LIMIT 1");
		mysql_free_result(res);
	} catch(compsky::mysql::except::SQLExec& e){
		this->logs("Error while attempting to access table ", tbl_name, ": ", e.what());
		abort();
	}
}


void DatabaseInfo::attempt_qry(const char* const qry) const {
	MYSQL_RES* res;
	MYSQL_ROW  row;
	try {
		compsky::mysql::query_buffer(this->mysql_obj, res, qry);
		mysql_free_result(res);
	} catch(compsky::mysql::except::SQLExec& e){
		this->logs("Error while attempting qry: ", e.what());
		abort();
	}
}


DatabaseInfo::DatabaseInfo(const char* const env_var_name,  const bool set_bools)
: bools{}
{
	using namespace compsky::utils; // for streq
	
	compsky::mysql::init_auth(buf, auth, getenv(env_var_name));
	compsky::mysql::login_from_auth(mysql_obj, auth);
	
	bool auto_reconnect = true;
	mysql_options(this->mysql_obj, MYSQL_OPT_RECONNECT, &auto_reconnect);
	
	if (not set_bools)
		return;
	
	this->bools[has_post_tbl] = true;
	
	MYSQL_RES* res;
	MYSQL_ROW  row;
	
	compsky::mysql::query_buffer(this->mysql_obj, res, "SHOW COLUMNS FROM user");
	const char* name;
	const char* type;
	const char* nullable;
	const char* key;
	const char* default_value;
	const char* extra;
	while(compsky::mysql::assign_next_row(res, &row, &name, &type, &nullable, &key, &default_value, &extra)){
		if (streq("full_name", name))
			this->bools[has_user_full_name_column] = true;
		else if (streq("verified", name))
			this->bools[has_user_verified_column] = true;
		else if (streq("n_followers", name))
			this->bools[has_n_followers_column] = true;
	}
	
	compsky::mysql::query_buffer(this->mysql_obj, res, "SHOW COLUMNS FROM post");
	while(compsky::mysql::assign_next_row(res, &row, &name, &type, &nullable, &key, &default_value, &extra)){
		if (streq("n_likes", name))
			this->bools[has_n_likes_column] = true;
	}
	
	compsky::mysql::query_buffer(this->mysql_obj, res, "SHOW TABLES");
	while(compsky::mysql::assign_next_row(res, &row, &name)){
		if (streq("follow", name))
			this->bools[has_follow_tbl] = true;
		else if (streq("cmnt", name))
			this->bools[has_cmnt_tbl] = true;
		else if (streq("post2mention", name))
			this->bools[has_post2mention_tbl] = true;
		else if (streq("cmnt2mention", name))
			this->bools[has_cmnt2mention_tbl] = true;
		else if (streq("hashtag", name))
			this->bools[has_hashtag_tbl] = true;
		else if (streq("post2like", name))
			this->bools[has_post2like_tbl] = true;
		else if (streq("cmnt2like", name))
			this->bools[has_cmnt2like_tbl] = true;
		else if (streq("user2tag", name))
			this->bools[has_user2tag_tbl] = true;
	}
	
	if (this->bools[has_cmnt_tbl]){
		compsky::mysql::query_buffer(this->mysql_obj, res, "SHOW COLUMNS FROM cmnt");
		while(compsky::mysql::assign_next_row(res, &row, &name, &type, &nullable, &key, &default_value, &extra)){
			if (streq("n_likes", name))
				this->bools[has_cmnt_n_likes_column] = true;
		}
	}
	
	this->attempt_to_access_tbl("post");
	this->attempt_to_access_tbl("user");
	if (this->bools[has_cmnt_tbl])
		this->attempt_to_access_tbl("cmnt");
	
}
