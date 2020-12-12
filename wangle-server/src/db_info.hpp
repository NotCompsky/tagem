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

#include "log.hpp"
#include "thread_pool.hpp"
#include <compsky/mysql/mysql.hpp>
#include <compsky/deasciify/a2n.hpp>


class DatabaseInfo : public ThreadPool<MYSQL*, DatabaseInfo> {
  public:
	constexpr static const size_t buf_sz = 512;
	char buf[buf_sz];
	compsky::mysql::MySQLAuth auth;
	
	enum B : unsigned {
		has_post_tbl,
		
		// User columns
		has_user_full_name_column,
		has_user_verified_column,
		has_n_followers_column,
		
		// Post columns
		has_n_likes_column,
		
		// Comment columns
		has_cmnt_n_likes_column,
		
		has_follow_tbl,
		has_cmnt_tbl,
		has_post2mention_tbl,
		has_cmnt2mention_tbl,
		has_hashtag_tbl,
		has_post2like_tbl,
		has_cmnt2like_tbl,
		has_user2tag_tbl,
		
		is_accessible_from_master_connection,
		
		COUNT
	};
	bool bools[COUNT];
	
	bool is_true(unsigned enum_indx) const {
		return this->bools[enum_indx];
	}
	
	void attempt_to_access_tbl(MYSQL* mysql_obj,  const char* const tbl_name) const;
	void attempt_qry(MYSQL* mysql_obj,  const char* const qry) const;
	
	const char* host() const {
		return auth.host;
	}
	const char* path() const {
		return auth.path;
	}
	const char* user() const {
		return auth.user;
	}
	const char* pwrd() const {
		return auth.pwrd;
	}
	const char* name() const {
		return auth.dbnm;
	}
	unsigned port() const {
		return a2n<unsigned>(auth.port);
	}
	void close();
	void test_is_accessible_from_master_connection(MYSQL* const master_connection,  char* buf);
	
	template<typename... Args>
	void logs(Args... args) const {
		static_log(this->name(), ": ", args...);
	}
	
	void query_buffer(MYSQL_RES*& res,  const char* const qry,  const size_t sz){
		MYSQL* const mysql_obj = this->get();
		try {
			compsky::mysql::query_buffer(mysql_obj, res, qry, sz);
		} catch(compsky::mysql::except::SQLExec& e){
			this->logs("Bad SQL: ", mysql_error(mysql_obj));
			throw(e);
		}
		this->free(mysql_obj);
	}
	
	void query_buffer(MYSQL_RES*& res,  const char* const qry){
		this->query_buffer(res, qry, strlen(qry));
	}
	
	void exec_buffer(const char* const qry,  const size_t sz){
		MYSQL* const mysql_obj = this->get();
		try {
			this->logs("exec_buffer");
			compsky::mysql::exec_buffer(mysql_obj, qry, sz);
		} catch(compsky::mysql::except::SQLExec& e){
			this->logs("Bad SQL: ", mysql_error(mysql_obj));
			throw(e);
		}
		this->free(mysql_obj);
	}
	
	void exec_buffer(const char* const qry){
		this->exec_buffer(qry, strlen(qry));
	}
	
	void new_obj(MYSQL*& mysql_obj) const;
	void kill_obj(MYSQL* mysql_obj) const;
	
	DatabaseInfo(const char* const env_var_name,  const bool set_bools);
};
