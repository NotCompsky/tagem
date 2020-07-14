#pragma once

#include <compsky/mysql/query.hpp>
#include <compsky/utils/streq.hpp>
#include <compsky/mysql/except.hpp>


struct DatabaseInfo {
	MYSQL* mysql_obj;
	constexpr static const size_t buf_sz = 512;
	char buf[buf_sz];
	char* auth[6];
	
	enum B : unsigned {
		has_post_tbl,
		
		// User columns
		has_user_full_name_column,
		has_user_verified_column,
		has_n_followers_column,
		
		// Post columns
		has_n_likes_column,
		
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
	
	const char* name() const {
		return auth[4];
	}
	MYSQL* connection() const {
		return this->mysql_obj;
	}
	void close(){
		mysql_close(mysql_obj);
		compsky::mysql::wipe_auth(buf, buf_sz);
	}
	void test_is_accessible_from_master_connection(MYSQL* const master_connection,  char* buf){
		try {
			MYSQL_RES* res;
			compsky::mysql::query(master_connection, res, buf, "SELECT 1 FROM ", this->name(), ".post LIMIT 1", '\0');
			mysql_free_result(res);
			this->bools[is_accessible_from_master_connection] = true;
		} catch(const compsky::mysql::except::SQLExec&){
			fprintf(stderr,  "Warning: Master is unable to access: %s\n",  this->name());
		}
	}
	DatabaseInfo(const char* const env_var_name,  const bool set_bools)
	: bools{}
	{
		using namespace compsky::utils; // for streq
		
		compsky::mysql::init_auth(buf, buf_sz, auth, getenv(env_var_name));
		compsky::mysql::login_from_auth(mysql_obj, auth);
		
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
	}
};
