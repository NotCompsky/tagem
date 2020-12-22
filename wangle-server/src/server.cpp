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
#define DEBUG

#define CACHE_CONTROL_HEADER "Cache-Control: max-age=" MAX_CACHE_AGE "\n"

#include "os.hpp"
#include "skip_to_body.hpp"
#include "qry.hpp"
#include "protocol.hpp"
#include "verify_str.hpp"
#include "str_utils.hpp"
#include "db_info.hpp"
#include "user_auth.hpp"
#include "proc.hpp"
#include "curl_utils.hpp"
#include "../../utils/src/thumbnailer.hpp"
#include "get_cookies.hpp"
#include "read_request.hpp"
#include "initialise_tagem_db.hpp"
#include "errors.hpp"
#include "handler_buf_pool.hpp"
#include "log.hpp"
#include "mimetype.hpp"
#include "fn_successness.hpp"
#ifdef ENABLE_SPREXER
# include "info_extractor.hpp"
#endif

#include <compsky/mysql/query.hpp>
#include <compsky/mysql/qryqry.hpp>
#include <compsky/mysql/alternating_qry.hpp>
#include <compsky/utils/is_str_dblqt_escaped.hpp>
#include <compsky/deasciify/a2n.hpp>
#include <compsky/deasciify/a2f.hpp>
#include <compsky/server/server.hpp>
#include <compsky/server/response_generation.hpp>
#include <compsky/os/write.hpp>

#include <mutex>

#include <filesystem> // for std::filesystem::copy_file

#ifndef NO_VIEW_DIR_FS
# include <openssl/md5.h>
#endif

#define cimg_display 0
#include <CImg.h>

#ifdef PYTHON
# include "python_stuff.hpp"
# include <rapidjson/document.h>
#endif

#include <magic.h>

/*
 * The following initial contents of YTDL_FORMAT are copyright TheFrenchGhostys (https://gitlab.com/TheFrenchGhosty)
 * Modified content from https://gitlab.com/TheFrenchGhosty/TheFrenchGhostys-YouTube-DL-Archivist-Scripts
 * GPL v3: https://github.com/TheFrenchGhosty/TheFrenchGhostys-YouTube-DL-Archivist-Scripts/raw/291b526c82f10b980c09ee5da9b432a039a1f0b0/LICENSE
 */
const char* YTDL_FORMAT = "(bestvideo[vcodec^=av01][height=720][fps>30]/bestvideo[vcodec^=vp9.2][height=720][fps>30]/bestvideo[vcodec^=vp9][height=720][fps>30]/bestvideo[vcodec^=avc1][height=720][fps>30]/bestvideo[height=720][fps>30]/bestvideo[vcodec^=av01][height=720]/bestvideo[vcodec^=vp9.2][height=720]/bestvideo[vcodec^=vp9][height=720]/bestvideo[vcodec^=avc1][height=720]/bestvideo[height=720]/bestvideo[vcodec^=av01][height<720][height>=480][fps>30]/bestvideo[vcodec^=vp9.2][height<720][height>=480][fps>30]/bestvideo[vcodec^=vp9][height<720][height>=480][fps>30]/bestvideo[vcodec^=avc1][height<720][height>=480][fps>30]/bestvideo[height<720][height>=480][fps>30]/bestvideo[vcodec^=av01][height<720][height>=480]/bestvideo[vcodec^=vp9.2][height<720][height>=480]/bestvideo[vcodec^=vp9][height<720][height>=480]/bestvideo[vcodec^=avc1][height<720][height>=480]/bestvideo[height<720][height>=480]/bestvideo[vcodec^=av01][height<720][height>=360][fps>30]/bestvideo[vcodec^=vp9.2][height<720][height>=360][fps>30]/bestvideo[vcodec^=vp9][height<720][height>=360][fps>30]/bestvideo[vcodec^=avc1][height<720][height>=360][fps>30]/bestvideo[height<720][height>=360][fps>30]/bestvideo[vcodec^=av01][height<720][height>=360]/bestvideo[vcodec^=vp9.2][height<720][height>=360]/bestvideo[vcodec^=vp9][height<720][height>=360]/bestvideo[vcodec^=avc1][height<720][height>=360]/bestvideo[height<720][height>=360]/bestvideo[vcodec^=av01][height<720][height>=240][fps>30]/bestvideo[vcodec^=vp9.2][height<720][height>=240][fps>30]/bestvideo[vcodec^=vp9][height<720][height>=240][fps>30]/bestvideo[vcodec^=avc1][height<720][height>=240][fps>30]/bestvideo[height<720][height>=240][fps>30]/bestvideo[vcodec^=av01][height<720][height>=240]/bestvideo[vcodec^=vp9.2][height<720][height>=240]/bestvideo[vcodec^=vp9][height<720][height>=240]/bestvideo[vcodec^=avc1][height<720][height>=240]/bestvideo[height<720][height>=240]/bestvideo[vcodec^=av01][height<720][height>=144][fps>30]/bestvideo[vcodec^=vp9.2][height<720][height>=144][fps>30]/bestvideo[vcodec^=vp9][height<720][height>=144][fps>30]/bestvideo[vcodec^=avc1][height<720][height>=144][fps>30]/bestvideo[height<720][height>=144][fps>30]/bestvideo[vcodec^=av01][height<720][height>=144]/bestvideo[vcodec^=vp9.2][height<720][height>=144]/bestvideo[vcodec^=vp9][height<720][height>=144]/bestvideo[vcodec^=avc1][height<720][height>=144]/bestvideo[height<720][height>=144]/bestvideo)+(bestaudio[acodec^=opus]/bestaudio)/best";
const char* FFMPEG_LOCATION = "/usr/bin/ffmpeg";

static
magic_t magique;

#define NULL_IMG_SRC "\"data:,\""

#define FILE_THUMBNAIL "IFNULL(IFNULL(f2tn.x, CONCAT('/i/f/', LOWER(HEX(f.md5_of_path)))), " NULL_IMG_SRC "),"
#define JOIN_FILE_THUMBNAIL "LEFT JOIN file2thumbnail f2tn ON f2tn.file=f.id "
#define DISTINCT_F2P_DB_AND_POST_IDS "IFNULL(GROUP_CONCAT(DISTINCT CONCAT(f2p.db,\":\",f2p.post),\"\"), \"\")"
#define DISTINCT_F2T_TAG_IDS "IFNULL(GROUP_CONCAT(DISTINCT f2t.tag),\"\")"
#define BLANK_IF_NULL(column) "IFNULL(" column ",\"\"),"
#define NULL_IF_NULL(column) "IFNULL(" column ",\"null\"),"
#define FILE_OVERVIEW_FIELDS(file_or_dir_id) \
	FILE_THUMBNAIL \
	file_or_dir_id "," \
	"f.name," \
	BLANK_IF_NULL("f.title") \
	NULL_IF_NULL("f.size") \
	"UNIX_TIMESTAMP(f.added_on)," \
	"f.t_origin," \
	NULL_IF_NULL("f.duration") \
	NULL_IF_NULL("f.w") \
	NULL_IF_NULL("f.h") \
	NULL_IF_NULL("f.views") \
	NULL_IF_NULL("f.likes") \
	NULL_IF_NULL("f.dislikes") \
	NULL_IF_NULL("f.fps") \
	DISTINCT_F2P_DB_AND_POST_IDS "," \
	DISTINCT_F2T_TAG_IDS ","

#define TRUE 1
#define FALSE 0

#define GET_PAGE_N(terminator) \
	const unsigned page_n = a2n<unsigned>(&s); \
	fflush(stdout); \
	if(*s != terminator) \
		return compsky::server::_r::not_found; \
	++s;

#define CHECK_FOR_EXPECT_100_CONTINUE_HEADER \
	if(SKIP_TO_HEADER(8,"Expect: ")(s) != nullptr) \
		return compsky::server::_r::expect_100_continue;

#define GET_COMMA_SEPARATED_INTS__NULLABLE(var_decl, var, var_length, str_name, terminating_char) \
	BOOST_PP_IF(var_decl, const char* const,) var  = get_comma_separated_ints(&str_name, terminating_char); \
	BOOST_PP_IF(var_decl, const size_t,) var_length  = (uintptr_t)str_name - (uintptr_t)var;

#define GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(var_decl, var, var_length, str_name, terminating_char) \
	GET_COMMA_SEPARATED_INTS__NULLABLE(var_decl, var, var_length, str_name, terminating_char) \
	if (var == nullptr) \
		return compsky::server::_r::not_found;

#define GET_USER \
	user_auth::User* user = user_auth::get_user(get_cookie(s, "username=")); \
	if (user == nullptr) \
		return compsky::server::_r::not_found;
#define GET_USER_ID \
	const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username=")); \
	if (user_id == user_auth::SpecialUserID::invalid) \
		return compsky::server::_r::not_found;
#define BLACKLIST_GUEST \
	if (user_id == user_auth::SpecialUserID::guest) \
		return compsky::server::_r::not_found;
#define GREYLIST_GUEST \
	if (user_id == user_auth::SpecialUserID::guest) \
		return compsky::server::_r::requires_login;

#define GET_DB_INFO \
	const unsigned db_id = a2n<unsigned>(&s); \
	const unsigned db_indx = external_db_id2indx(db_id); \
	if (db_indx == 0) \
		return compsky::server::_r::not_found; \
	DatabaseInfo& db_info = db_infos.at(db_indx);

#define GET_NUMBER(type,name) \
	const type name = a2n<type>(&s); \
	++s;

#define GET_NUMBER_NONZERO(type,name) \
	GET_NUMBER(type,name) \
	if (unlikely(name == 0)) \
		return compsky::server::_r::not_found;

#define GET_NUMBER_NONZERO_NOTCONDITION(type,name,condition) \
	GET_NUMBER(type,name) \
	if (unlikely((name == 0)) or (condition)) \
		return compsky::server::_r::not_found;

#define GET_FLOAT(type,name) \
	const type name = a2f<type>(&s); \
	++s;

#define SKIP_TO_BODY \
	s = skip_to_body(s); \
	if (unlikely(s == nullptr)) \
		return compsky::server::_r::not_found;

#define GET_FILE2_VAR_NAME(s) \
	const char* const file2_var_name = s; \
	while((*s != ' ') and (*s != 0)) \
		++s; \
	const size_t file2_var_name_len = (uintptr_t)s - (uintptr_t)file2_var_name; \
	/* No need to check for empty string - the later function does that*/ \
	GET_USER \
	if (unlikely(not matches__left_up_to_space__right_up_to_comma_or_null(file2_var_name, user->allowed_file2_vars_csv))) \
		return compsky::server::_r::not_found;


#define HIDDEN_TAGS_INNER(...) \
		"SELECT 1 " \
		"FROM tag2parent_tree t2pt " \
		"JOIN user2hidden_tag u2ht ON u2ht.user=" __VA_ARGS__ " AND u2ht.tag=t2pt.parent AND u2ht.max_depth>=t2pt.depth"
#define HIDDEN_TAGS(conditions, ...) \
	"JOIN tag2parent_tree t2pt ON " conditions " " \
	"JOIN user2hidden_tag u2ht ON u2ht.user=" __VA_ARGS__ " AND u2ht.tag=t2pt.parent AND u2ht.max_depth>=t2pt.depth "

#define NOT_HIDDEN_TAG(tag, user_id) \
	"NOT EXISTS(" HIDDEN_TAGS_INNER("", user_id, "") " WHERE (t2pt.id=", tag "))"

#define NOT_HIDDEN_TAG__GUEST(tag) \
	"NOT EXISTS(" HIDDEN_TAGS_INNER(GUEST_ID_STR) " WHERE (t2pt.id=" tag "))"

#ifdef LIMITS_WITHIN_GROUP_CONCATS
# define LIMIT1_WITHIN_GROUP_CONCAT "LIMIT 1"
# define SELECT_TAGS_INFOS_FROM_STUFF__DIRECTION "DESC"
# define NO_LIMITS_WITHIN_GROUP_CONCATS_WORKAROUND1 ""
# define NO_LIMITS_WITHIN_GROUP_CONCATS_WORKAROUND2 ""
#else
# define LIMIT1_WITHIN_GROUP_CONCAT "SEPARATOR ' '"
# define SELECT_TAGS_INFOS_FROM_STUFF__DIRECTION "ASC"
# define NO_LIMITS_WITHIN_GROUP_CONCATS_WORKAROUND1 "SUBSTRING_INDEX("
# define NO_LIMITS_WITHIN_GROUP_CONCATS_WORKAROUND2 ", ' ', -1)"
#endif

#define SELECT_TAGS_INFOS__BASE__SELECT \
	"SELECT " \
		"t.id," \
		"t.name," \
		NO_LIMITS_WITHIN_GROUP_CONCATS_WORKAROUND1 \
		"GROUP_CONCAT(IFNULL(p.thumbnail," NULL_IMG_SRC ") ORDER BY (1/(1+t2pt.depth))*(p.thumbnail IS NOT NULL) " SELECT_TAGS_INFOS_FROM_STUFF__DIRECTION " " LIMIT1_WITHIN_GROUP_CONCAT ")" \
		NO_LIMITS_WITHIN_GROUP_CONCATS_WORKAROUND2 \
		"," \
		"IFNULL(A.n,0)"

#define SELECT_TAGS_INFOS__BASE__BODY(...) \
	"FROM tag t " \
 	"JOIN tag2parent_tree t2pt ON t2pt.id=t.id " \
	"JOIN tag p ON p.id=t2pt.parent " \
	"LEFT JOIN(" \
		"SELECT f2t.tag, COUNT(*) AS n " \
		"FROM file2tag f2t " \
		/* NOTE: MySQL doesn't seem to optimise this often - usually faster if duplicate the limitations within this subquery */ \
		"JOIN file f ON f.id=f2t.file " \
		"JOIN dir d ON d.id=f.dir " \
		"WHERE f2t.tag IN(" \
			__VA_ARGS__ \
		")" \
		"AND " NOT_DISALLOWED_FILE("f.id", "f.dir", "d.device", user_id) \
		"GROUP BY tag" \
	")A ON A.tag=t.id "
#define SELECT_TAGS_INFOS_FROM_STUFF(...) \
	SELECT_TAGS_INFOS__BASE__SELECT \
	SELECT_TAGS_INFOS__BASE__BODY(__VA_ARGS__)
#define SELECT_TAGS_INFOS__WITH_T_AND_DESCR__FROM_STUFF(...) \
	SELECT_TAGS_INFOS__BASE__SELECT "," \
		"t.t_origin," \
		"t.t_ended," \
		"t.description " \
	SELECT_TAGS_INFOS__BASE__BODY(__VA_ARGS__)

#define WHERE_TAGS_INFOS(...) \
	"WHERE t.id IN(" __VA_ARGS__ ")" \
	"AND (t2pt.depth=0 OR p.thumbnail IS NOT NULL)" \
	"AND " NOT_DISALLOWED_TAG("t.id", user_id) \
	"AND " NOT_HIDDEN_TAG("t.id", user_id) \
	/* "AND p.id NOT IN" USER_DISALLOWED_TAGS(user_id)  Unnecessary */
#define TAGS_INFOS(...) \
	SELECT_TAGS_INFOS_FROM_STUFF(__VA_ARGS__) \
	WHERE_TAGS_INFOS(__VA_ARGS__) \
	"GROUP BY t.id "
#define TAGS_INFOS__WITH_T_AND_DESCR(...) \
	SELECT_TAGS_INFOS__WITH_T_AND_DESCR__FROM_STUFF(__VA_ARGS__) \
	WHERE_TAGS_INFOS(__VA_ARGS__) \
	"GROUP BY t.id "
#define TAGS_INFOS__WTH_DUMMY_WHERE_THING(...) \
	/* See NOTE #dkgja */ \
	SELECT_TAGS_INFOS_FROM_STUFF(__VA_ARGS__) \
	WHERE_TAGS_INFOS(__VA_ARGS__) \
	"AND t.id>0 " \
	"GROUP BY t.id "

#define GREYLIST_USERS_WITHOUT_PERMISSION(field_name) \
	if (not this->get_last_row_from_qry<bool>("SELECT id FROM user WHERE id=", user_id, " AND " field_name)) \
		return compsky::server::_r::unauthorised;

std::vector<DatabaseInfo> db_infos;

static
unsigned db_indx2id[128] = {};

unsigned external_db_id2indx(const unsigned id){
	for (size_t i = 0;  i < sizeof(db_indx2id);  ++i){
		if (id == db_indx2id[i])
			return i;
	}
	return 0;
}

static
std::vector<const char*> file2_variables;

static
std::vector<const char*> tables_referencing_file_id;

static
std::vector<const char*> left_join_unique_name_for_each_file2_var;

static
std::vector<const char*> select_unique_name_for_each_file2_var;

static
std::vector<uint64_t> connected_local_devices;
static
std::string connected_local_devices_str = "";

const char* CACHE_DIR = nullptr;
size_t CACHE_DIR_STRLEN;

const char* FILES_GIVEN_REMOTE_DIR = nullptr;
const char* EXTRACT_YTDL_ERA_SCRIPT_PATH = nullptr;

uint64_t TAG__PART_OF_FILE__ID;

static bool regenerate_mimetype_json = true;
static bool regenerate_device_json = true;
static bool regenerate_protocol_json = true;

uint64_t UPLOADER_TAG_ID;


namespace rapidjson {
uint64_t get_int(const rapidjson::Value& v,  const char* k){
	return v.HasMember(k) and not v[k].IsNull() ? v[k].GetInt() : 0;
}

float get_flt(const rapidjson::Value& v,  const char* k){
	return v.HasMember(k) and not v[k].IsNull() ? v[k].GetFloat() : 0;
}

const char* get_str(const rapidjson::Value& v,  const char* k,  const char* default_value = 0){
	return v.HasMember(k) and not v[k].IsNull() ? v[k].GetString() : default_value;
}
}

namespace _f {
	constexpr static Zip<2> zip2;
	constexpr static compsky::asciify::flag::Escape3 esc3;
}

namespace atomic_signal {
	static
	std::atomic<bool> stop_updating_video_metadatas = false;
}


namespace compsky {
namespace server {

std::vector<std::string> banned_client_addrs;

namespace _r {
	static const char* mimetype_json;
	static const char* external_db_json;
	static const char* protocols_json;
	static const char* devices_json;
	static const char* protocol_json;
	
	std::mutex mimetype_json_mutex;
	std::mutex external_db_json_mutex;
	std::mutex protocols_json_mutex;
	std::mutex devices_json_mutex;
	std::mutex protocol_json_mutex;
}
}
}

constexpr size_t handler_req_buf_sz = 4096 * 2;
static
HandlerBufPool handler_buf_pool;

class TagemResponseHandler : public compsky::server::ResponseGeneration {
 public:
	TagemResponseHandler()
	{
		this->buf = handler_buf_pool.get();
		this->reset_buf_index();
	}
	
	~TagemResponseHandler(){
		handler_buf_pool.free(this->buf);
	}
	
	std::string_view handle_request_2(boost::array<char, handler_req_buf_sz>& req_buffer,  const size_t n_bytes_of_first_req_buffer){
		char* str = req_buffer.data();
		str[n_bytes_of_first_req_buffer] = 0; // WARNING: Cannot handle n_bytes_of_first_req_buffer==handler_req_buf_sz - but neither can any of the following code, yet.
		--str;
		#include "auto-generated/auto__server-determine-response.hpp"
		return compsky::server::_r::not_found;
	}
	
	void handle_request(boost::array<char, handler_req_buf_sz>& req_buffer,  const size_t n_bytes_of_first_req_buffer,  std::vector<boost::asio::const_buffer>& response_buffers){
		const std::string_view v = this->handle_request_2(req_buffer, n_bytes_of_first_req_buffer);
		response_buffers.push_back(boost::asio::const_buffer(v.data(), v.size()));
	}
	
  private:
	template<size_t i = 0>
	void mysql_query_buf_db_by_id(DatabaseInfo& db_info,  const char* const _buf,  const size_t _buf_sz){
		db_info.query_buffer(this->res[i], _buf, _buf_sz);
	}
	
	void mysql_exec_buf_db_by_id(DatabaseInfo& db_info,  const char* const _buf,  const size_t _buf_sz){
		db_info.exec_buffer(_buf, _buf_sz);
	}
	
	void mysql_exec_buf(){
		this->mysql_exec_buf_db_by_id(db_infos.at(0), this->buf, this->buf_indx());
	}
	
	void mysql_exec_buf(const char* const _buf,  const size_t _buf_sz){
		this->mysql_exec_buf_db_by_id(db_infos.at(0), _buf, _buf_sz);
	}
	
	void mysql_exec_buf(const char* const _buf){
		this->mysql_exec_buf_db_by_id(db_infos.at(0), _buf, std::char_traits<char>::length(_buf));
	}
	
	template<size_t i = 0>
	void mysql_query_buf(const char* const _buf,  const size_t _buf_sz){
		this->mysql_query_buf_db_by_id<i>(db_infos.at(0), _buf, _buf_sz);
	}
	
	template<size_t i = 0>
	void mysql_query_buf(const char* const _buf){
		this->mysql_query_buf_db_by_id<i>(db_infos.at(0), _buf, std::char_traits<char>::length(_buf));
	}
	
	template<size_t i = 0>
	void mysql_query_using_buf(){
		this->mysql_query_buf<i>(this->buf, this->buf_indx());
	}
	
	void mysql_exec_using_buf_db_by_id(DatabaseInfo& db_info){
		this->mysql_exec_buf_db_by_id(db_info, this->buf, this->buf_indx());
	}
	
	void mysql_exec_using_buf(){
		this->mysql_exec_using_buf_db_by_id(db_infos.at(0));
	}
	
	template<size_t i = 0,  typename... Args>
	void mysql_query_db_by_id(DatabaseInfo& db_info,  Args... args){
		char* const itr_init = this->itr;
		this->asciify(args...);
		this->mysql_query_buf_db_by_id<i>(db_info, itr_init, (uintptr_t)this->itr - (uintptr_t)itr_init);
		this->itr = itr_init;
	}
	
	template<typename... Args>
	void mysql_exec_db_by_id(DatabaseInfo& db_info,  Args... args){
		char* const itr_init = this->itr;
		this->asciify(args...);
		this->mysql_exec_buf_db_by_id(db_info, itr_init, (uintptr_t)this->itr - (uintptr_t)itr_init);
		this->itr = itr_init;
	}
	
	template<size_t i = 0,  typename... Args>
	void mysql_query(Args... args){
		char* const itr_init = this->itr;
		this->asciify(args...);
		this->mysql_query_buf_db_by_id<i>(db_infos.at(0),  itr_init,  (uintptr_t)this->itr - (uintptr_t)itr_init);
		this->itr = itr_init;
	}
	
	template<typename... Args>
	void mysql_exec(Args... args){
		char* const itr_init = this->itr;
		this->asciify(args...);
		this->mysql_exec_buf_db_by_id(db_infos.at(0),  itr_init,  (uintptr_t)this->itr - (uintptr_t)itr_init);
		this->itr = itr_init;
	}
	
	template<typename... Args>
	void log(Args&&... args){
		/*char* const itr_init = this->itr;
		this->asciify(args..., '\n');
		compsky::os::write_n_bytes(STDERR_FILE_ID, itr_init, (uintptr_t)this->itr - (uintptr_t)itr_init);
		this->itr = itr_init;*/
	}
	
	bool user_can_access_dir(const UserIDIntType user_id, const uint64_t dir_id){
		return this->get_last_row_from_qry<bool>("SELECT 1 FROM dir d WHERE d.id=", dir_id, " AND " NOT_DISALLOWED_DIR("d.id", "d.device", user_id));
	}
	
	std::string_view parse_qry(const char* s){
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("exec_qry")
		SKIP_TO_BODY
		
		this->itr = this->buf;
		const char tbl_alias = s[0];
		
		const auto selected_field = sql_factory::parse_into(this->itr, s, connected_local_devices_str, user_id);
		if (unlikely(selected_field == sql_factory::selected_field::INVALID))
			return compsky::server::_r::post_not_necessarily_malicious_but_invalid;
		
		this->mysql_query_buf<0>(this->buf, strlen(this->buf)); // strlen used because this->itr is not set to the end
		this->reset_buf_index();
		
		if (selected_field == sql_factory::selected_field::URL_AND_TITLE__MARKDOWN){
			this->asciify('"');
			const char* url;
			const char* title;
			while(this->mysql_assign_next_row(&url, &title)){
				this->asciify('[', _f::esc3, '"', '[', ']', title, ']', '(', _f::esc3, '(', ')', '"', url, ')', "  \\n");
				// WARNING: This does not escape special unicode's "surrogate pairs" of quotes, for instance “
				// This causes JSON issues because JSON, beyond all reasoning, decides that this unicode "Left Double Quotation Mark" terminates a string opened by an ASCII double quotation mark.
				// This is not patched because knowing unicode there's probably a million such cases of visually similar quotation marks to patch.
			}
			this->asciify('"');
			return this->get_buf_as_string_view();
		}
		
		if (selected_field == sql_factory::selected_field::DELETE_LOCAL_FILES){
			const char* path;
			std::vector<uint64_t> file_ids;
			file_ids.reserve(compsky::mysql::n_results<size_t>(this->res[0]));
			uint64_t file_id;
			while(this->mysql_assign_next_row(&file_id, &path)){
				if (not os::del_file(path))
					file_ids.push_back(file_id);
			}
			this->mysql_exec(
				"UPDATE file "
				"SET status=66 "
				"WHERE id IN (0",
					_f::zip2, file_ids.size(), ',', file_ids,
				")"
			);
			return compsky::server::_r::post_ok;
		}
		
		if (selected_field != sql_factory::selected_field::X_ID)
			this->begin_json_response();
		
		const char* row = nullptr;
		if ((selected_field == sql_factory::selected_field::LIST) or (selected_field == sql_factory::selected_field::CHECK_LOCAL_FILES)){
			this->asciify('"');
			while(this->mysql_assign_next_row(&row)){
				if (
					((selected_field == sql_factory::selected_field::LIST) and true) or
					((selected_field == sql_factory::selected_field::CHECK_LOCAL_FILES) and os::file_exists(row))
				)
					this->asciify(_f::esc, '"', row, "\\n");
			}
			this->asciify('"');
			return this->get_buf_as_string_view();
		} else {
			while(this->mysql_assign_next_row(&row))
				this->asciify(row, ',');
		}
		
		if ((selected_field == sql_factory::selected_field::COUNT) or (selected_field == sql_factory::selected_field::TOTAL_SIZE) or (selected_field == sql_factory::selected_field::TOTAL_VIEWS)){
			--this->itr;
			return this->get_buf_as_string_view();
		}
		
		if (row == nullptr)
			// No results
			return compsky::server::_r::EMPTY_JSON_LIST;
		
		this->itr[-1] = 0; // Overwrite trailing comma
		
		return this->X_given_ids<true>(tbl_alias, user_id, 0, this->buf);
	}
	
	std::string_view file_thumbnail(const char* md5hex){
		if (*md5hex == ' ')
			return compsky::server::_r::invalid_file;
		
		for (auto i = 0;  i < 32;  ++i){
			if (not is_valid_hex_char(md5hex[i]))
				return compsky::server::_r::not_found;
		}
		this->reset_buf_index();
		this->asciify(
			CACHE_DIR,
			_f::strlen, md5hex, 32,
			".png"
		);
		*this->itr = 0;
		
		const size_t sz = os::get_file_sz(this->buf);
		if (unlikely(sz == 0)){
			this->log("No such file thumbnail: ", this->buf);
			return compsky::server::_r::invalid_file;
		}
		
		compsky::os::ReadOnlyFile f(this->buf);
		
		this->reset_buf_index();
		this->asciify(
			HEADER__RETURN_CODE__OK
			HEADER__CONTENT_TYPE__PNG
			CACHE_CONTROL_HEADER
			"Content-Length: ", sz, '\n',
			'\n'
		);
		
		f.read_into_buf(itr, sz);
		*(itr + sz) = 0;
		
		return std::string_view(this->buf,  sz + (uintptr_t)itr - (uintptr_t)this->buf);
	}
	
	std::string_view get_all_file_names_given_dir_id(const char* s){
		GET_NUMBER_NONZERO(uint64_t,id)
		
		// TODO: Add user permissions filter
		
		this->mysql_query<0>(
			"SELECT name "
			"FROM file "
			"WHERE dir=", id
		);
		
		this->begin_json_response();
		this->init_json_rows<0>(
			this->itr,
			compsky::server::_r::flag::arr,
			compsky::server::_r::flag::quote_and_json_escape // name
		);
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view dir_info(const char* s){
		GET_NUMBER_NONZERO(uint64_t,id)
		GET_USER_ID
		
		this->begin_json_response();
		this->asciify('[');
		
		this->mysql_query<0>(
			"SELECT d.name "
			"FROM dir d "
			"WHERE d.id=", id, " "
			  "AND " NOT_DISALLOWED_DIR("d.id", "d.device", user_id)
		);
		if (not this->init_json_rows<0>(
			this->itr,
			compsky::server::_r::flag::arr,
			compsky::server::_r::flag::quote_and_json_escape // name
		))
			return compsky::server::_r::unauthorised;
		this->asciify(',');
		
		// List of all parent directories
		this->mysql_query<0>(
			"SELECT d.id, d.name "
			"FROM dir d "
			"JOIN dir2parent_tree dt ON dt.parent=d.id "
			"WHERE dt.id=", id, " "
			  "AND " NOT_DISALLOWED_DIR("d.id", "d.device", user_id)
			"ORDER BY depth DESC"
		);
		this->init_json_rows<0>(
			this->itr,
			compsky::server::_r::flag::arr,
			compsky::server::_r::flag::quote_no_escape, // id_str
			compsky::server::_r::flag::quote_and_json_escape // name
		);
		this->asciify(',');
		
		// List of all IMMEDIATE child directories
		this->mysql_query<0>(
			"SELECT d.id, d.name "
			"FROM dir d "
			"WHERE d.parent=", id, " "
			  "AND " NOT_DISALLOWED_DIR("d.id", "d.device", user_id)
			"ORDER BY name ASC"
		);
		this->init_json_rows<0>(
			this->itr,
			compsky::server::_r::flag::arr,
			compsky::server::_r::flag::quote_no_escape, // id_str
			compsky::server::_r::flag::quote_and_json_escape // name
		);
		this->asciify(',');
		
		this->mysql_query<0>(
			"SELECT tag "
			"FROM dir2tag "
			"WHERE dir=", id
			// No tags are blacklisted, otherwise the directory would have been rejected above
		);
		this->init_json_rows<0>(
			this->itr,
			compsky::server::_r::flag::arr,
			compsky::server::_r::flag::quote_no_escape // tag id
		);
		
		this->asciify(']');
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
	}
	
	template<typename... Args>
	std::string_view ytdl_eras(const UserIDIntType user_id,  const uint64_t dest_dir,  const char* const eras,  const size_t eras_len,  Args... file_name_args){
		this->mysql_query<0>(
			"SELECT "
				"CONCAT(d.full_path, f.name),"
				"e.start,"
				"e.end,"
				"CONCAT(d2.full_path,", file_name_args..., "),"
				"CONCAT(\"Part of file: \",f.id)"
			"FROM era e "
			"JOIN file f ON f.id=e.file "
			"JOIN dir d ON d.id=f.dir "
			"JOIN dir d2 ON d2.id=", dest_dir, " "
			"WHERE e.id IN (", _f::strlen, eras, eras_len, ")"
			  "AND " NOT_DISALLOWED_ERA("e.id", "f.id", "d.id", "d.device", user_id)
			  "AND " NOT_DISALLOWED_DIR("d2.id", "d2.device", user_id)
		);
		
		const char* url = nullptr;
		const char* era_start;
		const char* era_end;
		const char* dest;
		const char* tag_name;
		this->mysql_assign_next_row<0>(&url, &era_start, &era_end, &dest, &tag_name);
		if (unlikely(url == nullptr))
			return compsky::server::_r::not_found;
		
		std::vector<const char*> args;
		args.reserve(1 + 1 + 3 * compsky::mysql::n_results<size_t>(this->res[0]) + 1);
		
		args[0] = EXTRACT_YTDL_ERA_SCRIPT_PATH;
		args[1] = dest;
		size_t i = 1;
		std::string tag_ids = "0";
		do {
			this->add_t_to_db(user_auth::SpecialUserID::admin, TAG__PART_OF_FILE__ID, std::string_view(tag_name, strlen(tag_name)));
			this->mysql_query<1>("SELECT id FROM tag WHERE name=\"", tag_name, "\" LIMIT 1");
			const char* tag_id_str;
			tag_ids += ",";
			while(this->mysql_assign_next_row<1>(&tag_id_str))
				tag_ids += tag_id_str;
			
#ifdef PYTHON__NOTIMPLEMENTEDYET
#else
			args[++i] = era_start;
			args[++i] = era_end;
			args[++i] = url;
#endif
		} while(this->mysql_assign_next_row<0>(&url, &era_start, &era_end, &dest, &tag_name));
		args.back() = nullptr;
		
		if (proc::exec(60,  args.data(),  STDOUT_FILENO,  nullptr,  0))
			return compsky::server::_r::server_error;
		
		this->add_file_or_dir_to_db('f', nullptr, user_id, tag_ids.data(), tag_ids.size(), dest, strlen(dest), 0, false, false);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__ytdl_eras(const char* s){
		GET_NUMBER_NONZERO(uint64_t,dest_dir)
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, eras, eras_len, s, ' ')
		GET_USER_ID
		SKIP_TO_BODY
		const char* const dest_file_name = s;
		
		return this->ytdl_eras(
			user_id,
			dest_dir,
			eras, eras_len,
			'"', _f::esc, '"', dest_file_name, '"'
		);
	}
	
	std::string_view post__ytdl_era(const char* s){
		GET_NUMBER_NONZERO(uint64_t,dest_dir)
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, eras, eras_len, s, ' ')
		GET_USER_ID
		
		return this->ytdl_eras(
			user_id,
			dest_dir,
			eras, eras_len,
			"f.id, '@', e.start, '-', e.end, '.mkv'"
		);
	}
	
	uint64_t get_dir_id_given_path__add_if_necessary(const UserIDIntType user_id,  const char* const path){
		if (unlikely(this->add_file_or_dir_to_db('d', nullptr, user_id, "0", 1, path, strlen(path), 0, false, false) != FunctionSuccessness::ok))
			return 0;
		return this->get_last_row_from_qry<uint64_t>("SELECT id FROM dir WHERE full_path=\"", _f::esc, '"', path, "\" LIMIT 1");
	}
	
	std::string_view files_given_dir__filesystem(const char* s){
		GET_PAGE_N(' ')
#ifndef NO_VIEW_DIR_FS
		GET_USER_ID
		GREYLIST_GUEST
		SKIP_TO_BODY
		
		const char* const dir_path = s;
		
		std::array<uint8_t, 16> hash;
		
		const bool is_local = os::is_local_file_or_dir(dir_path);
		
		os::dir_handler_typ dir;
		
		if (is_local){
			dir = os::open_dir(dir_path);
			if (unlikely(dir == nullptr))
				return compsky::server::_r::server_error;
		}
		
		// Determine if files exist in the database - if so, supply all the usual data
		const SQLQuery qry1(
			this,
			db_infos.at(0),
			"SELECT f.name "
			"FROM file f "
			"JOIN dir d ON d.id=f.dir "
			"WHERE d.name=\"", _f::esc, '"', dir_path, "\" "
			"UNION "
			"SELECT f.name "
			"FROM file_backup f "
			"JOIN dir d ON d.id=f.dir "
			"WHERE d.name=\"", _f::esc, '"', dir_path, "\" "
			// WARNING: No limits. Directories should aim to avoid having too many files in each (low thousands) to mitigate malicious requests
		);
		
		this->begin_json_response();
		this->asciify("[");
		
		if (not is_local){
			char* const _buf = this->itr;
			char* _itr = _buf;
			compsky::asciify::asciify(_itr, page_n, '\0');
#ifdef PYTHON
			const bool failed = python::view_remote_dir(_buf, dir_path);
#else
			const char* args[] = {FILES_GIVEN_REMOTE_DIR, _buf, dir_path, nullptr};
			const bool failed = (FILES_GIVEN_REMOTE_DIR != nullptr) and proc::exec(60,  args,  STDOUT_FILENO,  this->itr,  HANDLER_BUF_SZ - this->buf_indx());
#endif
			if (unlikely(failed))
				this->move_itr_to_trailing_null();
		} else {
		
		this->begin_json_response();
		const uint64_t dir_id = this->get_dir_id_given_path__add_if_necessary(user_id, dir_path);
		this->asciify("[\"", dir_id, "\",[");
		os::dirent_typ e;
		unsigned min = 100 * page_n;
		unsigned indx = 0;
		unsigned count = 100;
		while (os::get_next_item_in_dir(dir, e) and (count != 0)){
			const char* const ename = e->d_name;
			
			if (os::is_dir(e))
				continue;
			
			if (compsky::mysql::in_results<0>(ename, qry1.res))
				// If ename is equal to a string in the 2nd column of the results, it has already been recorded
				continue;
			
			if (++indx <= min)
				continue;
			
			--count;
			
			this->md5_hash_local_file(hash.data(), dir_path, ename);
			
			char file_path[4096];
			compsky::asciify::asciify(file_path, dir_path, ename, '\0');
			
			size_t file_size;
			time_t file_time;
			os::get_file_size_and_ctime(file_path, file_size, file_time);
			this->asciify(
				// Should be equivalent to asciify_file_info
				'[',
					"\"/i/f/", _f::lower_case, _f::hex, hash, "\"", ',', // thumbnail
					0, ',',                                              // ID
					'"', _f::esc, '"', ename,   '"', ',',                // name
					'"', '"', ',',                                       // title
					'"', file_size, '"', ',',                            // size
					'"', file_time, '"', ',',                            // t_added
					'0', ',',                                            // t_origin
					'0', ',',                                            // duration
					'0', ',',                                            // w
					'0', ',',                                            // h
					'0', ',',                                            // views
					'0', ',',                                            // likes
					'0', ',',                                            // dislikes
					'0', ',',                                            // fps
					'"', '"', ',',                                       // DB and post IDs
					'"', '"',                                            // f2t tag IDs
				']',
				','
			);
		}
		os::close_dir(dir);
		
		}
		
		if (this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify("],{}]"); // Empty tags dictionary
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
#else
		return compsky::server::_r::not_found;
#endif
	}
	
	std::string_view file_info(const char* s){
		GET_NUMBER_NONZERO(uint64_t,id)
		GET_USER
		const UserIDIntType user_id = user->id;
		
		const size_t n = user->allowed_file2_vars.size();
		
		
		this->reset_buf_index();
		this->begin_json_response();
		this->asciify('[');
		
		--this->itr; // Removes the previous open bracket. This is because the following SQL query has only ONE response - an array would be appropriate if there were more
		this->mysql_query<0>(
			"SELECT "
				FILE_OVERVIEW_FIELDS("f.dir")
				"f.mimetype,"
				"IFNULL(f.description,\"\"),"
				"CONCAT(\"0\"", _f::n_elements, n, select_unique_name_for_each_file2_var, ")"
			"FROM file f "
			"JOIN dir d ON d.id=f.dir "
			"LEFT JOIN file2tag f2t ON f2t.file=f.id "
			"LEFT JOIN file2post f2p ON f2p.file=f.id "
			JOIN_FILE_THUMBNAIL,
			_f::zip3, n, "LEFT JOIN file2", user->allowed_file2_vars, left_join_unique_name_for_each_file2_var,
			"WHERE f.id=", id, " "
			  "AND " NOT_DISALLOWED_FILE("f.id", "f.dir", "d.device", user_id)
			"GROUP BY f.id"
		);
		if(not this->init_json_rows<0>(
			this->itr,
			compsky::server::_r::flag::arr,
			compsky::server::_r::flag::quote_no_escape, // md5_hash,
			compsky::server::_r::flag::quote_no_escape, // dir_id,
			compsky::server::_r::flag::quote_and_json_escape, // file_name,
			compsky::server::_r::flag::quote_and_json_escape, // file_title,
			compsky::server::_r::flag::quote_no_escape, // file_sz,
			compsky::server::_r::flag::no_quote, // file_added_timestamp,
			compsky::server::_r::flag::no_quote, // file_origin_timestamp,
			compsky::server::_r::flag::no_quote, // duration,
			compsky::server::_r::flag::no_quote, // w,
			compsky::server::_r::flag::no_quote, // h,
			compsky::server::_r::flag::no_quote, // views,
			compsky::server::_r::flag::no_quote, // likes,
			compsky::server::_r::flag::no_quote, // dislikes,
			compsky::server::_r::flag::no_quote, // fps,
			compsky::server::_r::flag::quote_no_escape, // external_db_and_post_ids,
			compsky::server::_r::flag::quote_no_escape, // tag_ids,
			compsky::server::_r::flag::no_quote, // mimetype,
			compsky::server::_r::flag::quote_and_json_escape, // description
			compsky::server::_r::flag::quote_no_escape // file2_values
		))
			// No results - probably because the user hasn't the permission to view the file
			return compsky::server::_r::not_found;
		--this->itr; // Removes the previous close bracket. This is because the following SQL query has only ONE response - an array would be appropriate if there were more
		this->asciify(',');
		
		
		this->mysql_query<0>(
			"SELECT "
				"e.id,"
				"e.start,"
				"e.end,"
				"IFNULL(GROUP_CONCAT(e2t.tag),\"\")"
			"FROM era e "
			"JOIN file f ON f.id=e.file "
			"LEFT JOIN era2tag e2t ON e2t.era=e.id "
			// NOTE: Each era should be associated with at least one tag, and this may be assumed in other parts of this server. However, I believe in second chances, and the client should be allowed to rectify their error by having these abominations rendered even without tags.
			"WHERE e.file=", id, " "
			  "AND " NOT_DISALLOWED_ERA("e.id", "0", "0", "0", user_id) // File has already been filtered for permissions
			"GROUP BY e.id "
			"HAVING COUNT(*)" // Ensure we don't get an empty result - (NULL,NULL,NULL) - when it really means we have no results
		);
		this->init_json_rows<0>(
			this->itr,
			compsky::server::_r::flag::arr,
			compsky::server::_r::flag::quote_no_escape, // era ID
			compsky::server::_r::flag::no_quote, // era_start,
			compsky::server::_r::flag::no_quote, // era_end,
			compsky::server::_r::flag::quote_no_escape // era_tag_ids
		);
		this->asciify(',');
		
		
		this->mysql_query<0>(
			"SELECT f.dir, f.name, f.mimetype "
			"FROM file_backup f "
			"JOIN dir d ON d.id=f.dir "
			"WHERE f.file=", id, " "
			  "AND " NOT_DISALLOWED_DIR("f.dir", "d.device", user_id)
		);
		this->init_json_rows<0>(
			this->itr,
			compsky::server::_r::flag::arr,
			compsky::server::_r::flag::quote_no_escape, // backup_dir_id,
			compsky::server::_r::flag::quote_and_json_escape, // backup_file_name,
			compsky::server::_r::flag::no_quote // backup_mimetype
		);
		this->asciify(',');
		
		
		this->mysql_query<0>(
			"SELECT d.id, d.full_path, d.device "
			"FROM dir d "
			"JOIN("
				"SELECT DISTINCT dir "
				"FROM file "
				"WHERE id=", id, " "
				"UNION "
				"SELECT DISTINCT dir "
				"FROM file_backup "
				"WHERE file=", id,
			")A ON A.dir=d.id "
			"WHERE " NOT_DISALLOWED_DIR("d.id", "d.device", user_id)
		);
		this->init_json_rows<0>(
			this->itr,
			compsky::server::_r::flag::dict,
			compsky::server::_r::flag::quote_no_escape, // dir_id,
			compsky::server::_r::flag::quote_and_json_escape, // dir_name,
			compsky::server::_r::flag::quote_no_escape // device_id
		);
		this->asciify(',');
		
		
		this->mysql_query<0>(
			"SELECT "
				"DISTINCT t.id,"
				"t.name "
			"FROM tag t "
			"JOIN era2tag e2t ON e2t.tag=t.id "
			"JOIN era e ON e.id=e2t.era "
			"WHERE e.file=", id, " "
			  "AND " NOT_HIDDEN_TAG("t.id", user_id)
			  "AND " NOT_DISALLOWED_ERA("e.id", "0", "0", "0", user_id) // File table already filtered
			"UNION "
			"SELECT "
				"DISTINCT t.id,"
				"t.name "
			"FROM tag t "
			"JOIN box2tag b2t ON b2t.tag=t.id "
			"JOIN box b ON b.id=b2t.box "
			"WHERE b.file=", id, " "
			  "AND " NOT_HIDDEN_TAG("t.id", user_id)
			  "AND " NOT_DISALLOWED_TAG("b.id", user_id)
		);
		this->init_json_rows<0>(
			this->itr, compsky::server::_r::flag::dict,
			compsky::server::_r::flag::quote_no_escape, // tag_id,
			compsky::server::_r::flag::quote_and_json_escape // tag_name
		);
		this->asciify(',');
		
		
		this->mysql_query<0>(
			"SELECT "
				"b.id,"
				"b.frame_n,"
				"b.x,"
				"b.y,"
				"b.w,"
				"b.h,"
				"GROUP_CONCAT(b2t.tag)"
			"FROM box b "
			"JOIN box2tag b2t ON b2t.box=b.id "
			"WHERE b.file=", id, " "
			  "AND " NOT_DISALLOWED_TAG("b2t.tag", user_id)
			"GROUP BY b.id"
		);
		this->init_json_rows<0>(
			this->itr, compsky::server::_r::flag::arr,
			compsky::server::_r::flag::quote_no_escape, // box ID
			compsky::server::_r::flag::no_quote,        // frame number
			compsky::server::_r::flag::no_quote,        // x
			compsky::server::_r::flag::no_quote,        // y
			compsky::server::_r::flag::no_quote,        // w
			compsky::server::_r::flag::no_quote,        // h
			compsky::server::_r::flag::quote_no_escape  // tag IDs
		);
		this->asciify(']');
		
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view tags_given_file(const char* s){
		GET_NUMBER_NONZERO(uint64_t,id)
		
		this->mysql_query<0>(
			"SELECT "
				"tag "
			"FROM file2tag "
			"WHERE file=", id, " "
			"LIMIT 1000"
		);
		
		this->write_json_list_response_into_buf<0>(compsky::server::_r::flag::quote_no_escape);
		
		return this->get_buf_as_string_view();
	}
	
	template<size_t i = 0,  typename ArrOrDict>
	void asciify_tags_arr_or_dict(char*& itr,  const ArrOrDict f_arr_or_dict){
		compsky::asciify::asciify(itr, compsky::server::_r::opener_symbol(f_arr_or_dict));
		this->asciify_json_response_rows<i>(
			itr,
			f_arr_or_dict,
			compsky::server::_r::flag::quote_no_escape, // id,
			compsky::server::_r::flag::quote_and_json_escape, // name,
			compsky::server::_r::flag::quote_and_escape, // thumb,
			compsky::server::_r::flag::no_quote // count
		);
		compsky::asciify::asciify(itr, compsky::server::_r::closer_symbol(f_arr_or_dict));
	}
	
	template<size_t i = 0,  typename ArrOrDict>
	void asciify_tags_arr_or_dict(const ArrOrDict f_arr_or_dict){
		return this->asciify_tags_arr_or_dict<i>(this->itr, f_arr_or_dict);
	}
	
	void asciify_file_info(char*& itr){
		compsky::asciify::asciify(itr, "[\"0\",");
		this->init_json_rows<0>(
			itr,
			compsky::server::_r::flag::arr,
			compsky::server::_r::flag::quote_no_escape, // md5_hex thumbnail
			compsky::server::_r::flag::quote_no_escape, // file_id
			compsky::server::_r::flag::quote_and_json_escape, // file name
			compsky::server::_r::flag::quote_and_json_escape, // file title
			compsky::server::_r::flag::quote_no_escape, // file size
			compsky::server::_r::flag::no_quote, // file added timestamp
			compsky::server::_r::flag::no_quote, // file origin timestamp
			compsky::server::_r::flag::no_quote, // duration
			compsky::server::_r::flag::no_quote, // w
			compsky::server::_r::flag::no_quote, // h
			compsky::server::_r::flag::no_quote, // views
			compsky::server::_r::flag::no_quote, // likes
			compsky::server::_r::flag::no_quote, // dislikes
			compsky::server::_r::flag::no_quote, // fps
			compsky::server::_r::flag::quote_no_escape, // external db and post IDs
			compsky::server::_r::flag::quote_no_escape, // tag IDs CSV
			compsky::server::_r::flag::no_quote, // era start
			compsky::server::_r::flag::no_quote // era end
		);
		compsky::asciify::asciify(itr, ',');
		this->asciify_tags_arr_or_dict<1>(itr, compsky::server::_r::flag::dict);
		compsky::asciify::asciify(itr, "]");
		*itr = 0;
	}
	
	void asciify_file_info(){
		this->asciify_file_info(this->itr);
	}
	
	std::string_view post__record_files(const char* s){
		GET_NUMBER_NONZERO_NOTCONDITION(uint64_t,dir_id,s[-1]!=' ')
		CHECK_FOR_EXPECT_100_CONTINUE_HEADER
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("add_files")
		SKIP_TO_BODY
		
		const char* const body = s;
		
		this->reset_buf_index();
		this->asciify(
			"INSERT INTO file "
			"(user,dir,name)"
			"VALUES"
		);
		while(true){
			const char* const file_name_begin = s;
			if(unlikely(not compsky::utils::is_str_dblqt_escaped(s, ',', '\0')))
				return compsky::server::_r::not_found;
			const size_t file_name_len = (uintptr_t)s - (uintptr_t)file_name_begin;
			this->asciify('(', user_id, ',', dir_id, ',', _f::strlen, file_name_begin, file_name_len, ')', ',');
			if (*s == 0)
				break;
			++s; // Skip comma
		}
		if (unlikely(this->last_char_in_buf() != ','))
			// No file names were provided
			return compsky::server::_r::not_found;
		--this->itr; // Remove trailing comma
		this->asciify("ON DUPLICATE KEY UPDATE user=user");
		*this->itr = 0;
		
		this->mysql_exec_buf();
		
		// Now return a map of name to ID
		this->mysql_query<0>("SELECT name, id FROM file WHERE dir=", dir_id, " AND name IN(", body, ")");
		this->reset_buf_index();
		this->init_json<0>(
			&this->itr,
			compsky::server::_r::flag::dict,
			nullptr,
			compsky::server::_r::flag::quote_no_escape, // name,
			compsky::server::_r::flag::quote_no_escape // id
		);
		return this->get_buf_as_string_view();
	}
	
	struct SQLQuery {
		TagemResponseHandler* const thees;
		MYSQL_RES* res;
		MYSQL_ROW row;
		bool not_freed_results;
		
		template<typename... Args>
		SQLQuery(TagemResponseHandler* const _thees,  DatabaseInfo& db_info,  Args... args)
		: thees(_thees)
		, not_freed_results(true)
		{
			char* _itr = thees->itr;
			MYSQL* const mysql_obj = db_info.get();
			compsky::mysql::query(mysql_obj, this->res, _itr, args...);
			db_info.free(mysql_obj);
		}
		
		~SQLQuery(){
			if (not_freed_results)
				mysql_free_result(this->res);
		}
		
		template<typename... Args>
		bool assign_next_row(Args... args){
			return this->not_freed_results = compsky::mysql::assign_next_row(this->res, &this->row, args...);
		}
		
		template<typename... Args>
		bool assign_next_row__no_free(Args... args){
			return compsky::mysql::assign_next_row__no_free(this->res, &this->row, args...);
		}
	};
	
	class StringFromSQLQuery_DB {
	 private:
		TagemResponseHandler* const thees;
		MYSQL_RES* res;
	 public:
		const char* value;
		
		template<typename... Args>
		StringFromSQLQuery_DB(TagemResponseHandler* _thees,  DatabaseInfo& db_info,  Args... args)
		: thees(_thees)
		, value(nullptr)
		{
			MYSQL_RES* const _res_orig = thees->res[0];
			thees->mysql_query_db_by_id<0>(db_info, args...);
			thees->mysql_assign_next_row<0>(&this->value);
			this->res = thees->res[0];
			thees->res[0] = _res_orig;
		}
		
		~StringFromSQLQuery_DB(){
			if (this->value != nullptr)
				mysql_free_result(this->res);
		}
	};
	
	class StringFromSQLQuery : public StringFromSQLQuery_DB {
	 public:
		template<typename... Args>
		StringFromSQLQuery(TagemResponseHandler* _thees,  Args... args)
		: StringFromSQLQuery_DB(_thees, db_infos.at(0), args...)
		{}
	};
	
	std::string_view post__create_file(const char* s){
		uint64_t file_id = a2n<uint64_t>(&s); // NOTE: Can be zero
		++s;
		if(s[-1] != '/')
			return compsky::server::_r::not_found;
		GET_NUMBER_NONZERO_NOTCONDITION(uint64_t,dir_id,s[-1]!='/')
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, ' ')
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("create_files")
		
		if (unlikely(not this->user_can_access_dir(user_id, dir_id)))
			return compsky::server::_r::not_found;
		
		SKIP_TO_BODY
		
		const char* const file_name = s;
		const char* const b4_file_contents = skip_to(s, '\n');
		const size_t file_name_length = (uintptr_t)b4_file_contents - (uintptr_t)file_name;
		
		if(unlikely(b4_file_contents == nullptr))
			return compsky::server::_r::not_found;
		
		const char* const file_contents = b4_file_contents + 1;
		
		if (file_id != 0)
			// Update an existing file
			return compsky::server::_r::not_implemented_yet;
		
		const StringFromSQLQuery path(this, "SELECT CONCAT(full_path, \"", _f::esc, '"', _f::strlen, file_name_length, file_name, "\") FROM dir WHERE id=", dir_id);
		// NOTE: Guaranteed to be a good dir_id
		const bool is_local_dir = os::is_local_file_or_dir(path.value);
		
		if (is_local_dir){
			// If the directory is non-local, the body of the request is set as the description rather than saved to a file.
			// This is designed to store small snippets - code snippets, quotes, etc.
			
			if (unlikely(os::file_exists(path.value)))
				return 
					HEADER__RETURN_CODE__SERVER_ERR
					"\n"
					"File already exists"
				;
			
			this->log("Creating file: ", path.value);
			if (unlikely(os::write_to_file(path.value, file_contents, strlen(file_contents))))
				return compsky::server::_r::server_error;
		}
		
		while(file_id == 0){
			// NOTE: Might never be called if file existed in DB
			const unsigned mimetype_id = (is_local_dir) ? 17 : 0; // "text/plain"
			const char* const description = (is_local_dir) ? "" : file_contents;
			
			/*
			 * WARNING: Probably violates law of least surprise.
			 * If file exists on filesystem: nothing happens
			 * Else if file exists on DB:    file is created on FS and DB, and tagged
			 * Else if file not exist:       file is created on DB, and tagged
			 */
			this->mysql_exec(
				"INSERT INTO file "
				"(dir,user,mimetype,name,description)"
				"VALUES(",
					dir_id, ',',
					user_id, ',',
					mimetype_id, ',',
					'"', _f::esc, '"', _f::strlen, file_name_length, file_name, '"', ',',
					'"', _f::esc, '"', description, '"',
				")"
			);
			
			file_id = this->get_last_row_from_qry<uint64_t>("SELECT id FROM file WHERE dir=", dir_id, " AND name=\"", _f::esc, '"', _f::strlen, file_name_length, file_name, "\" LIMIT 1");
		}
		
		this->add_tags_to_files(
			user_id,
			std::string_view(tag_ids, tag_ids_len),
			"AND f.id=", file_id // File already permission checked
		);
		
		return compsky::server::_r::post_ok;
	}
	
	template<typename... Args>
	void swap_file_with_a_backup(const uint64_t file_id,  const uint64_t backup_dir_id,  Args... backup_file_name_args){
		this->mysql_exec(
			"UPDATE file f "
			"JOIN dir d2 ON d2.id=", backup_dir_id, " "
			"JOIN file_backup f2 ON ("
				    "f.id=", file_id, " "
				"AND f2.file=f.id "
				"AND f2.dir=d2.id "
				"AND f2.name=", backup_file_name_args..., " "
			")"
			"SET "
				// Dummy actions to set temporary variables
				"f.dir=@dir:=f.dir,"
				"f.name=@name:=f.name,"
				"f.mimetype=@mimetype:=f.mimetype,"
				"f.user=@user:=f.user,"
				
				"f.dir=f2.dir,"
				"f.name=f2.name,"
				"f.mimetype=f2.mimetype,"
				"f.user=f2.user,"
				
				"f2.dir=@dir,"
				"f2.name=@name,"
				"f2.mimetype=@mimetype,"
				"f2.user=@user"
		);
	}
	
	std::string_view replace_file_path_and_set_old_path_as_backup(const char* s){
		GET_NUMBER_NONZERO(uint64_t, file_id)
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("backup_files")
		SKIP_TO_BODY
		
		const char* const new_path__file_name = s;
		
		uint64_t new_dir_id;
		const auto rc = this->add_file_or_dir_to_db__w_parent_dir_id<0>(new_dir_id, 'f', nullptr, user_id, "0", 1, new_path__file_name, strlen(new_path__file_name), 0, false, false);
		if (unlikely(rc != FunctionSuccessness::ok))
			return (rc == FunctionSuccessness::malicious_request) ? compsky::server::_r::not_found : compsky::server::_r::server_error;
		
		const uint64_t new_file_id = this->get_last_row_from_qry<uint64_t>(
			"SELECT f.id "
			"FROM file f "
			"JOIN dir d ON d.id=f.dir "
			"WHERE f.name=SUBSTR(\"", _f::esc, '"', new_path__file_name, "\",LENGTH(d.full_path)+1)"
			  "AND d.id=", new_dir_id
		);
		
		assert(new_file_id != 0);
		
		this->merge_files(user_id, file_id, new_file_id);
		// NOTE: It might be that the new 'original source' is already in our database. If that is the case, that file is 'found' by add_file_or_dir_to_db__w_parent_dir_id, and that file is merged in the above step. Hence all the fiddling with file2tag etc. is necessary.
		
		this->swap_file_with_a_backup(
			file_id,
			new_dir_id,
			"SUBSTR(\"", _f::esc, '"', new_path__file_name, "\",LENGTH(d2.full_path)+1)"
		);
		// NOTE: The merge would fail here if a file weren't allowed to have multiple backups in the same directory.
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view external_user_posts(const char* s,  const unsigned required_db_info_bool_indx,  const char* const tbl_name,  const char* const col_name){
		GET_DB_INFO
		if (unlikely(not db_info.is_true(required_db_info_bool_indx)))
			return compsky::server::_r::not_found;
		++s;
		GET_NUMBER_NONZERO(uint64_t,external_user_id)
		GET_USER_ID
		
		const StringFromSQLQuery_DB post_ids(
			this,
			db_info,
			"SELECT GROUP_CONCAT(DISTINCT ", col_name, ")"
			"FROM ", tbl_name, " "
			"WHERE user=", external_user_id
		);
		if (post_ids.value == nullptr){
			// mysql_assign_next_row always returns a single row due to the GROUP_CONCAT function
			return compsky::server::_r::EMPTY_JSON_LIST;
		}
		
		this->reset_buf_index();
		const StringFromSQLQuery file_ids(
			this,
			"SELECT GROUP_CONCAT(f2p.file)"
			"FROM file2post f2p "
			"JOIN file f ON f.id=f2p.file "
			"JOIN dir d ON d.id=f.dir "
			"WHERE f2p.post IN (", post_ids.value, ")"
			  "AND " NOT_DISALLOWED_FILE("f2p.file", "f.dir", "d.device", user_id)
			"LIMIT " TABLE_LIMIT
		);
		
		if (file_ids.value == nullptr){
			// mysql_assign_next_row always returns a single row due to the GROUP_CONCAT function
			return compsky::server::_r::EMPTY_JSON_LIST;
		}
		
		this->begin_json_response();
		this->asciify(
			"["
				"\"", file_ids.value, "\""
			"]"
		);
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view external_user_info(const char* s){
		// Viewing the user's liked posts and comments etc. is in a separate function
		
		GET_DB_INFO
		++s;
		GET_NUMBER_NONZERO(uint64_t,user_id)
		
		SQLQuery qry1(
			this,
			db_info,
			"SELECT "
				"u.name,",
				(db_info.is_true(DatabaseInfo::has_user_full_name_column)) ? "IFNULL(u.full_name,\"\")," : "\"\",",
				(db_info.is_true(DatabaseInfo::has_user_verified_column)) ? "u.verified," : "FALSE,",
				(db_info.is_true(DatabaseInfo::has_n_followers_column)) ? "IFNULL(u.n_followers,0)," : "0,",
				(db_info.is_true(DatabaseInfo::has_user2tag_tbl)) ? "IFNULL(GROUP_CONCAT(u2t.tag),\"\")" : "\"\"", " "
			"FROM user u ",
			(db_info.is_true(DatabaseInfo::has_user2tag_tbl)) ? "LEFT JOIN user2tag u2t ON u2t.user=u.id" : "", " "
			"WHERE u.id=", user_id
		);
		
		this->begin_json_response();
		this->asciify('[');
		const char* name;
		const char* full_name;
		const char* is_verified;
		const char* n_followers;
		const char* tag_ids;
		qry1.assign_next_row(&name, &full_name, &is_verified, &n_followers, &tag_ids);
		// mysql_assign_next_row always returns a single row due to the GROUP_CONCAT function
		if (name == nullptr){
			// No ushc user
			return compsky::server::_r::not_found;
		}
		this->asciify(
			'"', name, '"', ',',
			'"', _f::esc, '"', full_name, '"', ',',
			is_verified, ',',
			n_followers, ',',
			'"', tag_ids, '"'
		);
		this->asciify(']');
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view external_cmnt_rm(const char* s){
		GET_DB_INFO
		++s;
		GET_NUMBER_NONZERO(uint64_t,cmnt_id)
		
		if (not db_info.is_true(DatabaseInfo::has_cmnt_tbl))
			return compsky::server::_r::not_found;
		
		this->mysql_exec_db_by_id(
			db_info,
			"UPDATE cmnt "
			"SET content=NULL "
			"WHERE id=", cmnt_id
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view external_post_likes(const char* s){
		GET_DB_INFO
		++s;
		GET_NUMBER_NONZERO(uint64_t,post_id)
		
		if (not db_info.is_true(DatabaseInfo::has_post2like_tbl))
			return compsky::server::_r::EMPTY_JSON_LIST;
		
		this->mysql_query_db_by_id<0>(
			db_info,
			"SELECT "
				"u.id,"
				"u.name "
			"FROM user u "
			"JOIN post2like p2l ON p2l.user=u.id "
			"WHERE p2l.post=", post_id
		);
		this->write_json_list_response_into_buf<0>(
			compsky::server::_r::flag::quote_no_escape, // user_id
			compsky::server::_r::flag::quote_no_escape // username
		);
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view external_post_info(const char* s){
		// Data comes in two parts: data excluding comments, and then comments
		
		// TODO: Restrict with user permissions
		
		GET_DB_INFO
		++s;
		GET_NUMBER_NONZERO(uint64_t,post_id)
		
		char* const _buf_plus_offset = this->buf + 300;
		char* _itr_plus_offset = _buf_plus_offset;
		// Reserve the first part of this->buf for writing SQL queries, and use the rest for writing the response.
		
		compsky::asciify::asciify(_itr_plus_offset, compsky::server::_r::json_init);
		compsky::asciify::asciify(_itr_plus_offset, '[');
		
		{
			this->mysql_query_db_by_id(
				db_info,
				"SELECT "
					"p.user,"
					"p.t,",
					(db_info.is_true(DatabaseInfo::has_n_likes_column)) ? "IFNULL(p.n_likes,\"\")," : "0,",
					"u.name,"
					"IFNULL(p.text,\"\") "
				"FROM post p "
				"JOIN user u ON u.id=p.user "
				"WHERE p.id=", post_id
			);
			
			if (not this->init_json_rows(
				_itr_plus_offset,
				compsky::server::_r::flag::arr,
				compsky::server::_r::flag::quote_no_escape, // user,
				compsky::server::_r::flag::no_quote, // timestamp,
				compsky::server::_r::flag::no_quote, // n_likes,
				compsky::server::_r::flag::quote_no_escape, // username,
				compsky::server::_r::flag::quote_and_json_escape // text
			))
				return compsky::server::_r::not_found;
			compsky::asciify::asciify(_itr_plus_offset, ',');
		}
		
		if (db_info.is_true(DatabaseInfo::has_cmnt_tbl)){
			this->mysql_query_db_by_id(
				db_info,
				"SELECT "
					"c.id,"
					"IFNULL(c.parent,0),",
					(db_info.is_true(DatabaseInfo::has_cmnt_n_likes_column)) ? "c.n_likes" : "0", " AS likes,"
					"c.user,"
					"c.t,"
					"u.name,"
					"IFNULL(c.content,\"\") "
				"FROM cmnt c "
				"JOIN user u ON u.id=c.user "
				"WHERE c.post=", post_id, " "
				  "AND c.content IS NOT NULL "
					// Ignore comments deleted through web interface
					// TODO: Display some deleted comments that have some non-deleted children. Maybe SQL script to replace NULL with empty string?
				"ORDER BY c.parent ASC, likes DESC" // Put parentless comments first
			);
			
			this->init_json_rows(
				_itr_plus_offset,
				compsky::server::_r::flag::arr,
				compsky::server::_r::flag::quote_no_escape, // id,
				compsky::server::_r::flag::quote_no_escape, // parent,
				compsky::server::_r::flag::quote_no_escape, // n_likes,
				compsky::server::_r::flag::quote_no_escape, // user,
				compsky::server::_r::flag::no_quote, // timestamp,
				compsky::server::_r::flag::quote_no_escape, // username,
				compsky::server::_r::flag::quote_and_json_escape // content
			);
		} else {
			compsky::asciify::asciify(_itr_plus_offset, "[]");
		}
		compsky::asciify::asciify(_itr_plus_offset, ']');
		
		*_itr_plus_offset = 0;
		
		return std::string_view(_buf_plus_offset,  (uintptr_t)_itr_plus_offset - (uintptr_t)_buf_plus_offset);
	}
	
	std::string_view files_given_tag(const char* s){
		GET_PAGE_N('/')
		GET_NUMBER_NONZERO(uint64_t,id)
		GET_USER_ID
		
		return
			JoinArgsWrapper<const char*>
			::WhereArgsWrapper<const char*, uint64_t, const char*>
			::OrderArgsWrapper<const char*>
			::files_given_X__string_view(
				this,
				user_id,
				page_n,
				"0,0",
				"f.id",
				"LEFT JOIN file2tag f2t ON f2t.file=f.id",
				"AND EXISTS (SELECT file FROM file2tag WHERE file=f.id AND tag=", id, ")",
				"ORDER BY NULL",
				"SELECT DISTINCT file FROM file2tag WHERE tag=", id
			);
	}
	
	std::string_view dirs_given_tag(const char* s){
		GET_PAGE_N('/')
		GET_NUMBER_NONZERO(uint64_t,tag_id)
		GET_USER_ID
		
		return this->X_given_ids<false>('d', user_id, page_n, "SELECT dir FROM dir2tag WHERE tag=", tag_id);
	}
	
	template<typename... Args>
	void eras_w_file_infos_given_ids(char*& itr,  const UserIDIntType user_id,  const unsigned page_n,  Args... ids_args){
		// WARNING: files_given_X uses both res[0] and res[1]
		JoinArgsWrapper<const char*, Args..., const char*>
		::template WhereArgsWrapper<const char*, Args..., const char*>
		::template OrderArgsWrapper<const char*, Args..., const char*>
		::files_given_X(
			this,
			user_id,
			page_n,
			"e.start,e.end",
			"e.id",
			"JOIN era e ON e.file=f.id "
			"LEFT JOIN("
				"SELECT era, tag "
				"FROM era2tag "
				"WHERE era IN(", ids_args..., ")"
				"UNION "
				"SELECT e.id, f2t.tag "
				"FROM file2tag f2t "
				"JOIN era e ON e.file=f2t.file"
			")f2t ON f2t.era=e.id",
			"AND e.id IN(", ids_args..., ")",
			"ORDER BY FIELD(e.id,", ids_args..., ")",
			"SELECT DISTINCT file FROM era WHERE id IN(", ids_args..., "))UNION SELECT DISTINCT tag FROM era2tag WHERE era IN(", ids_args...
		);
		
		this->asciify_file_info(itr);
	}
	
	template<typename... Args>
	void files_given_ids(char*& itr,  const UserIDIntType user_id,  const unsigned page_n,  Args... ids_args){
		JoinArgsWrapper<const char*>
		::WhereArgsWrapper<const char*, Args..., const char*>
		::template OrderArgsWrapper<const char*, Args..., const char*>
		::files_given_X(
			this,
			user_id,
			page_n,
			"0,0",
			"f.id",
			"LEFT JOIN file2tag f2t ON f2t.file=f.id",
			"AND f.id IN(", ids_args..., ")",
			"ORDER BY FIELD(f.id,", ids_args..., ")",
			ids_args...
		);
		
		this->asciify_file_info(itr);
	}
	
	template<size_t i = 0>
	std::string_view get__tag_info__given_id(const char* s){
		GET_NUMBER_NONZERO(uint64_t,tag_id)
		GET_USER_ID
		this->mysql_query<i>(
			TAGS_INFOS__WITH_T_AND_DESCR("", tag_id, "")
		);
		this->reset_buf_index();
		this->begin_json_response(this->itr);
		this->asciify('[');
		this->asciify_json_response_rows<i>(
			this->itr,
			compsky::server::_r::flag::arr,
			compsky::server::_r::flag::quote_no_escape, // id,
			compsky::server::_r::flag::quote_and_json_escape, // name,
			compsky::server::_r::flag::quote_and_escape, // thumb,
			compsky::server::_r::flag::no_quote, // count
			compsky::server::_r::flag::no_quote, // t_origin
			compsky::server::_r::flag::no_quote, // t_ended
			compsky::server::_r::flag::quote_and_json_escape // description
		);
		this->asciify(']');
		return this->get_buf_as_string_view();
	}
	
	template<size_t i,  bool is_ordered_by_field,  typename... Args>
	void tags_given_ids(char*& itr,  const UserIDIntType user_id,  const unsigned page_n,  Args... ids_args){
		this->mysql_query<i>(
			TAGS_INFOS("", ids_args..., "")
			"ORDER BY ", is_ordered_by_field ? "FIELD(t.id," : "t.name -- ", ids_args..., ")\n"
			"LIMIT 100 "
			"OFFSET ", 100*page_n
		);
		this->asciify_tags_arr_or_dict<i>(itr, compsky::server::_r::flag::arr);
	}
	
	template<bool is_ordered_by_field,  typename... Args>
	void dirs_given_ids(char*& itr,  const UserIDIntType user_id,  const unsigned page_n,  Args... ids_args){
		this->mysql_query<1>(
			TAGS_INFOS("SELECT DISTINCT tag FROM dir2tag WHERE dir IN(", ids_args..., ")")
		);
		this->mysql_query<0>(
			"SELECT "
				"d.id,"
				"d.name,"
				"d.device,"
				"IFNULL(GROUP_CONCAT(DISTINCT d2t.tag),\"\"),"
				"IFNUlL(COUNT(DISTINCT f.id),0)+IFNULL(COUNT(DISTINCT d_child.id),0) AS n "
			"FROM dir d "
			"LEFT JOIN dir2tag d2t ON d2t.dir=d.id "
			"LEFT JOIN file f ON f.dir=d.id "
			"LEFT JOIN dir d_child ON d_child.parent=d.id "
			"WHERE d.id IN (", ids_args..., ")"
			  "AND " NOT_DISALLOWED_FILE("f.id", "f.dir", "d.device", user_id)
			"GROUP BY d.id "
			"ORDER BY ", is_ordered_by_field ? "FIELD(d.id," : "d.name -- ", ids_args..., ")\n"
			"LIMIT 100 "
			"OFFSET ", 100*page_n
		);
		
		compsky::asciify::asciify(itr, '[');
		
		this->init_json_rows<0>(
			itr,
			compsky::server::_r::flag::arr,
			compsky::server::_r::flag::quote_no_escape, // id,
			compsky::server::_r::flag::quote_and_json_escape, // name,
			compsky::server::_r::flag::quote_no_escape, // device
			compsky::server::_r::flag::quote_no_escape, // tag IDs
			compsky::server::_r::flag::no_quote // count
		);
		compsky::asciify::asciify(itr, ',');
		
		this->asciify_tags_arr_or_dict<1>(itr, compsky::server::_r::flag::dict);
		
		compsky::asciify::asciify(itr, ']');
	}
	
	template<bool is_ordered_by_field,  typename... Args>
	std::string_view X_given_ids(const char tbl_alias,  const UserIDIntType user_id,  const unsigned page_n,  Args... ids_args){
		char* const itr_init = this->itr;
		this->begin_json_response(this->itr);
		switch(tbl_alias){
			case 'f':
				this->files_given_ids(this->itr, user_id, 0, ids_args...);
				break;
			case 'e':
				this->eras_w_file_infos_given_ids(this->itr, user_id, 0, ids_args...);
				break;
			case 'd':
				this->dirs_given_ids<is_ordered_by_field>(this->itr, user_id, 0, ids_args...);
				break;
			case 't':
				this->tags_given_ids<0, is_ordered_by_field>(this->itr, user_id, 0, ids_args...);
				break;
			default:
				abort();
		}
		*this->itr = 0;
		return std::string_view(itr_init,  (uintptr_t)this->itr - (uintptr_t)itr_init);
	}
	
	std::string_view get__X_given_ids(const char tbl_alias,  const char* s){
		GET_PAGE_N('/')
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, ids, ids_len, s, ' ')
		GET_USER_ID
		return this->X_given_ids<true>(tbl_alias, user_id, page_n, _f::strlen, ids, ids_len);
	}
	
	std::string_view get_parent_tags_of(const char* s){
		GET_PAGE_N('/')
		GET_NUMBER_NONZERO(uint64_t,tag_id)
		GET_USER_ID
		return this->X_given_ids<false>('t', user_id, page_n, "SELECT parent FROM tag2parent WHERE id=", tag_id);
	}
	
	std::string_view get_child_tags_of(const char* s){
		GET_PAGE_N('/')
		GET_NUMBER_NONZERO(uint64_t,tag_id)
		GET_USER_ID
		return this->X_given_ids<false>('t', user_id, page_n, "SELECT id FROM tag2parent WHERE parent=", tag_id);
	}
	
	template<size_t i = 0,  typename... Args>
	void qry_mysql_for_next_parent_dir(const UserIDIntType user_id,  const uint64_t child_dir_id,  Args... args){
		// This function is to be used to traverse the dir table from the deepest ancestor to the immediate parent
		this->mysql_query<i>(
			"SELECT d.id, LENGTH(d.name)"
			"FROM dir d "
			"WHERE LEFT(CAST(\"", _f::esc, '"', args..., "\" AS BINARY),LENGTH(name))=name "
			  "AND d.parent", (child_dir_id==0)?" IS NULL AND 0=":"=", child_dir_id, " "
			  "AND " NOT_DISALLOWED_DIR("d.id", "d.device", user_id)
			"ORDER BY LENGTH(d.name) DESC "
			"LIMIT 1"
		);
	}
	
	template<typename... JoinArgs>
	struct JoinArgsWrapper {
		template<typename... WhereArgs>
		struct WhereArgsWrapper {
			template<typename... OrderArgs>
			struct OrderArgsWrapper {
				template<typename... FileIDsArgs>
				static
				void files_given_X(
					TagemResponseHandler* thees,
					const UserIDIntType user_id,
					const unsigned page_n,
					const char* const select_fields,
					const char* const group_by,
					JoinArgs... join_args,
					WhereArgs... where_args,
					OrderArgs... order_args,
					FileIDsArgs... file_ids_args
				){
					thees->mysql_query<1>(
						TAGS_INFOS__WTH_DUMMY_WHERE_THING("SELECT DISTINCT tag FROM file2tag WHERE file IN(", file_ids_args..., ")")
					);
					thees->mysql_query<0>(
						"SELECT "
							FILE_OVERVIEW_FIELDS("f.id"),
							select_fields, " "
						"FROM file f "
						"JOIN dir d ON d.id=f.dir "
						JOIN_FILE_THUMBNAIL
						"LEFT JOIN file2post f2p ON f2p.file=f.id ",
						join_args..., " "
						"WHERE TRUE "
						  "AND " NOT_DISALLOWED_TAG( "f2t.tag", user_id)
						  "AND " NOT_DISALLOWED_FILE("f.id", "f.dir", "d.device", user_id),
						where_args..., " "
						"GROUP BY f.id ",
						order_args..., " "
						"LIMIT " TABLE_LIMIT " "
						"OFFSET ", 100*page_n
					);
				}
				
				template<typename... Args>
				static
				std::string_view files_given_X__string_view(TagemResponseHandler* thees,  Args... args){
					thees->begin_json_response();
					files_given_X(thees, args...);
					thees->asciify_file_info();
					return thees->get_buf_as_string_view();
				}
			};
		};
	};
	
	std::string_view files_given_dir(const char* s){
		GET_PAGE_N('/')
		GET_NUMBER_NONZERO(uint64_t,id)
		GET_USER_ID
		
		return
			JoinArgsWrapper<const char*>
			::WhereArgsWrapper<const char*, uint64_t>
			::OrderArgsWrapper<const char*>
			::files_given_X__string_view(
				this,
				user_id,
				page_n,
				"0,0",
				"f.id",
				"LEFT JOIN file2tag f2t ON f2t.file=f.id",
				"AND f.dir=", id,
				"ORDER BY NULL",
				"SELECT DISTINCT id FROM file WHERE dir=", id
			);
	}
	
	std::string_view files_given_value(const char* s){
		GET_PAGE_N('/')
		GET_FILE2_VAR_NAME(s)
		const auto user_id = user->id;
		
		this->mysql_query<1>(
			TAGS_INFOS("SELECT DISTINCT tag FROM file2tag WHERE file IN(SELECT DISTINCT file FROM file2", _f::strlen, file2_var_name, file2_var_name_len, ")")
		);
		this->mysql_query<0>(
			"SELECT "
				FILE_OVERVIEW_FIELDS("f.id")
				"0,"
				"0 "
			"FROM file f "
			"JOIN dir d ON d.id=f.dir "
			"JOIN file2", _f::strlen, file2_var_name, file2_var_name_len, " f2v ON f2v.file=f.id "
			"LEFT JOIN file2tag f2t ON f2t.file=f.id "
			JOIN_FILE_THUMBNAIL
			"LEFT JOIN file2post f2p ON f2p.file=f.id "
			"WHERE " NOT_DISALLOWED_FILE("f.id", "f.dir", "d.device", user->id)
			"GROUP BY f.id "
			"LIMIT " TABLE_LIMIT " "
			"OFFSET ", 100*page_n
			// No need to impose a limit - this is very quick
		);
		
		this->begin_json_response();
		this->asciify_file_info();
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view get_allowed_file2_vars_json(const char* s){
		GET_USER
		
		this->mysql_query<0>(
			"SELECT f2.id, name, f2.min, f2.max, f2.conversion "
			"FROM file2 f2 "
			"JOIN user2shown_file2 u2f2 ON u2f2.file2=f2.id "
			"WHERE u2f2.user=", user->id
		);
		
		this->reset_buf_index();
		this->asciify(compsky::server::_r::json_init);
		this->init_json_rows(
			this->itr,
			compsky::server::_r::flag::dict,
			compsky::server::_r::flag::quote_no_escape, // ID
			compsky::server::_r::flag::quote_no_escape, // name (name of SQL table, so no special characters)
			compsky::server::_r::flag::quote_no_escape, // min
			compsky::server::_r::flag::quote_no_escape, // max
			compsky::server::_r::flag::no_quote         // conversion ID
		);
		return this->get_buf_as_string_view();
	}
	
	std::string_view get_mimetype_json(const char* s){
		this->mysql_query_buf<0>("SELECT id, name FROM mimetype");
		
		std::unique_lock lock(compsky::server::_r::mimetype_json_mutex);
		if (unlikely(regenerate_mimetype_json)){
			// WARNING: Race condition since init_json uses global mysql objects
			// TODO: Eliminate race with mutex
			regenerate_mimetype_json = false;
			this->init_json<0>(
				nullptr,
				compsky::server::_r::flag::dict,
				&compsky::server::_r::mimetype_json,
				compsky::server::_r::flag::quote_no_escape, // id,
				compsky::server::_r::flag::quote_no_escape // name
			);
		}
		return compsky::server::_r::mimetype_json;
	}
	
	template<typename... Args>
	std::string_view select2(const char tbl_alias,  const UserIDIntType user_id,  Args... name_args){
		this->reset_buf_index();
		this->asciify(
			"SELECT X.id, X.", (tbl_alias=='d')?"full_path ":"name ",
			"FROM ", (tbl_alias=='d')?"dir":"tag", " X "
			"WHERE "
		);
		switch(tbl_alias){
			case 'd':
				this->asciify(NOT_DISALLOWED_DIR("X.id", "X.device", user_id));
				break;
			default: /*case 't'*/
				this->asciify(NOT_DISALLOWED_TAG("X.id", user_id));
				break;
		}
		this->asciify(
			  "AND ", (tbl_alias=='d')?"full_path ":"name ", name_args..., "\" "
			"LIMIT 20"
		); // TODO: Tell client if results have been truncated
		
		if (unlikely(this->itr[-11] == '\\'))
			// If the last double quote is escaped
			return compsky::server::_r::EMPTY_JSON_LIST;				
		
		try{
			this->mysql_query_using_buf();
		}catch(const compsky::mysql::except::SQLExec& e){
			// NOTE: This appears to cause the server to hang on all *subsequent* queries involving SQL.
			this->log(e.what());
			return compsky::server::_r::EMPTY_JSON_LIST;
		}
		if (unlikely(this->res[0] == nullptr))
			return compsky::server::_r::EMPTY_JSON_LIST;
		this->reset_buf_index();
		this->init_json<0>(
			&this->itr,
			compsky::server::_r::flag::dict,
			nullptr,
			compsky::server::_r::flag::quote_no_escape, // id
			compsky::server::_r::flag::quote_and_json_escape // name
		);
		return this->get_buf_as_string_view();
	}
	
	std::string_view select2_regex(const char tbl_alias,  const char* s){
		const char* const qry = s;
		
		GET_USER_ID
		
		return this->select2(tbl_alias, user_id, "REGEXP BINARY \"", _f::esc_dblqt, _f::unescape_URI_until_space, _f::upper_case, qry);
		// NOTE: AFAIK, in URLs, one should have spaces as %20 before the ? and + after.
		// However, select2 encodes spaces as %20, not +, even though they are passed as URL parameters.
	}
	
	std::string_view get_device_json(const char* s){
		GET_USER_ID
		if (user_id != user_auth::SpecialUserID::guest){
			this->mysql_query<0>(
				"SELECT D.id, D.name, D.protocol, D.embed_pre, D.embed_post "
				"FROM device D "
				"WHERE " NOT_DISALLOWED_DEVICE("D.id", user_id)
			);
			this->itr = this->buf;
			this->init_json<0>(
				&this->itr,
				compsky::server::_r::flag::dict,
				nullptr,
				compsky::server::_r::flag::quote_no_escape, // id,
				compsky::server::_r::flag::quote_and_json_escape, // name,
				compsky::server::_r::flag::quote_no_escape, // protocol,
				compsky::server::_r::flag::quote_and_escape, // embed_pre,
				compsky::server::_r::flag::quote_and_escape // embed_post
			);
			return this->get_buf_as_string_view();
		}
		
		std::unique_lock lock(compsky::server::_r::devices_json_mutex);
		if (unlikely(regenerate_device_json)){
			regenerate_device_json = false;
			this->mysql_query_buf<0>(
				"SELECT D.id, D.name, D.protocol, D.embed_pre, D.embed_post "
				"FROM device D "
				"WHERE " NOT_DISALLOWED_DEVICE__COMPILE_TIME("D.id", GUEST_ID_STR)
			);
			this->init_json<0>(
				nullptr,
				compsky::server::_r::flag::dict,
				&compsky::server::_r::devices_json,
				compsky::server::_r::flag::quote_no_escape, // id,
				compsky::server::_r::flag::quote_and_json_escape, // name,
				compsky::server::_r::flag::quote_no_escape, // protocol,
				compsky::server::_r::flag::quote_and_escape, // embed_pre,
				compsky::server::_r::flag::quote_and_escape // embed_post
			);
		}
		return compsky::server::_r::devices_json;
	}
	
	std::string_view get_exec_json(const char* s){
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("exec_unsafe_tasks")
		
		this->mysql_query<0>(
			"SELECT id, name, description, content "
			"FROM task"
		);
		this->itr = this->buf;
		this->init_json<0>(
			&this->itr,
			compsky::server::_r::flag::arr,
			nullptr,
			compsky::server::_r::flag::quote_no_escape, // id,
			compsky::server::_r::flag::quote_and_json_escape, // name,
			compsky::server::_r::flag::quote_and_json_escape, // description,
			compsky::server::_r::flag::quote_and_json_escape // content
		);
		return this->get_buf_as_string_view();
	}
	
	std::string_view exec_task(const char* s){
		GET_NUMBER_NONZERO(unsigned,task_id)
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("exec_unsafe_tasks")
		
		StringFromSQLQuery content(
			this,
			"SELECT content "
			"FROM task "
			"WHERE id=", task_id
		);
		
		if(content.value == nullptr)
			// User tried to execute a task they were not authorised to see
			return compsky::server::_r::not_found;
		
		const char* content_end = content.value;
		while(true){
			while((*content_end != 0) and (*content_end != ';'))
				++content_end;
			this->mysql_exec_buf(content.value,  (uintptr_t)content_end - (uintptr_t)content.value);
			if(*content_end == 0)
				break;
			content.value = ++content_end;
		}
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view edit_task(const char* s){
		GET_NUMBER_NONZERO(unsigned,task_id)
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("edit_tasks")
		SKIP_TO_BODY
		
		this->mysql_query<0>(
			"UPDATE task "
			"SET content=\"", _f::esc, '"', s, "\" "
			"WHERE id=", task_id
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view get_protocol_json(const char* s){
		std::unique_lock lock(compsky::server::_r::protocol_json_mutex);
		if (unlikely(regenerate_protocol_json)){
			regenerate_protocol_json = false;
			this->mysql_query_buf<0>("SELECT id, name, 0 FROM protocol");
			this->init_json<0>(
				nullptr,
				compsky::server::_r::flag::dict,
				&compsky::server::_r::protocol_json,
				compsky::server::_r::flag::quote_no_escape, // id,
				compsky::server::_r::flag::quote_and_json_escape, // name,
				compsky::server::_r::flag::no_quote // dummy  // To deliver it as id:[name] rather than id:name // TODO: Tidy
			);
		}
		return compsky::server::_r::protocol_json;
	}
	
	std::string_view get_user_json(const char* s){
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("edit_users")
		
		this->mysql_query_buf<0>(
			"SELECT "
				"u.id,"
				"u.name,"
				"CONCAT_WS("
					"\",\","
					"stream_files,"
					"exec_qry,"
					"exec_safe_sql_cmds,"
					"exec_unsafe_sql_cmds,"
					"exec_safe_tasks,"
					"exec_unsafe_tasks,"
					"edit_tasks,"
					"link_tags,"
					"unlink_tags,"
					"edit_tags,"
					"merge_files,"
					"backup_files,"
					"add_files,"
					"create_files,"
					"edit_names,"
					"add_eras,"
					"record_local_fs,"
					"edit_users"
				"),"
				"IFNULL(GROUP_CONCAT(u2t.tag),\"\")"
			"FROM user u "
			"LEFT JOIN user2blacklist_tag u2t ON u2t.user=u.id "
			"WHERE u.id != 0 "
			"GROUP BY u.id"
		);
		this->reset_buf_index();
		this->begin_json_response();
		this->asciify('[');
		
		this->init_json_rows<0>(
			this->itr,
			compsky::server::_r::flag::dict,
			compsky::server::_r::flag::quote_no_escape, // id,
			compsky::server::_r::flag::quote_and_json_escape, // name,
			compsky::server::_r::flag::quote_no_escape, // boolean permission values as integer CSV
			compsky::server::_r::flag::quote_no_escape // user2blacklist_tag id
		);
		this->asciify(',');
		
		this->mysql_query<0>(
			TAGS_INFOS("SELECT DISTINCT tag FROM user2blacklist_tag")
		);
		this->asciify_tags_arr_or_dict<0>(itr, compsky::server::_r::flag::dict);
		
		this->asciify(']');
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view post__add_user(const char* s){
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("edit_users")
		SKIP_TO_BODY
		
		const char* const username = s;
		
		this->mysql_exec(
			"INSERT INTO user"
			"(name)"
			"VALUES(",
				_f::esc, '"', username,
			")"
			"ON DUPLICATE KEY UPDATE id=id"
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__update_user_permission(const char* s){
		GET_NUMBER(unsigned,editing_user_of_id)
		GET_NUMBER(bool,new_value)
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("edit_users")
		SKIP_TO_BODY
		
		const char* const field_name = s;
		// WARNING: field_name is not verified
		
		this->mysql_exec(
			"UPDATE user "
			"SET ", field_name, "=", new_value?'1':'0', " "
			"WHERE id=", editing_user_of_id
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__rm_user_blacklisted_tag(const char* s){
		GET_NUMBER(unsigned,editing_user_of_id)
		GET_NUMBER(uint64_t,tag_id)
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("edit_users")
		SKIP_TO_BODY
		
		const char* const field_name = s;
		// WARNING: field_name is not verified
		
		this->mysql_exec(
			"DELETE FROM user2blacklist_tag "
			"WHERE user=", editing_user_of_id, " "
			  "AND tag=",  tag_id
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__add_user_blacklisted_tag(const char* s){
		GET_NUMBER(unsigned,editing_user_of_id)
		GET_NUMBER(uint64_t,tag_id)
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("edit_users")
		SKIP_TO_BODY
		
		const char* const field_name = s;
		// WARNING: field_name is not verified
		
		this->mysql_exec(
			"INSERT INTO user2blacklist_tag"
			"(user,tag)"
			"VALUES(",
				editing_user_of_id, ',',
				tag_id,
			")"
			"ON DUPLICATE KEY UPDATE user=user"
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view stream_file(const char* s){
		constexpr static const size_t block_sz = 1024 * 1024 * 10;
		constexpr static const size_t stream_block_sz = 1024 * 1024; // WARNING: Will randomly truncate responses, usually around several MiBs // TODO: Increase this buffer size.
		constexpr static const size_t room_for_headers = 1000;
		static_assert(HANDLER_BUF_SZ  >  block_sz + room_for_headers); // 1000 is to leave room for moving headers around
		
		GET_NUMBER_NONZERO(uint64_t, id)
		
		uint64_t dir_id = 0;
		--s;
		if(*s == '/'){
			++s; // Skip trailing slash
			dir_id = a2n<uint64_t>(s);
			if (unlikely(dir_id == 0))
				return compsky::server::_r::not_found;
		}
		
		size_t from;
		size_t to;
		const GetRangeHeaderResult rc = get_range(s, from, to);
		if (unlikely(rc == GetRangeHeaderResult::invalid)){
			return compsky::server::_r::not_found;
		}
		
		if (unlikely( (to != 0) and (to <= from) ))
			return compsky::server::_r::not_found;
		
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("stream_files")
		
		SQLQuery qry1(
			this,
			db_infos.at(0),
			"SELECT m.name, CONCAT(d.full_path, f", (dir_id==0)?"":"2", ".name) "
			"FROM file f ",
			(dir_id==0)?"":"JOIN file_backup f2 ON f2.file=f.id ",
			"JOIN dir d ON d.id=f", (dir_id==0)?"":"2", ".dir "
			"JOIN mimetype m ON m.id=f", (dir_id==0)?"":"2", ".mimetype "
			"WHERE f.id=", id, " "
			  "AND ", (dir_id==0)?"0=":"f2.dir=", dir_id, " "
			  "AND " NOT_DISALLOWED_FILE("f.id", "f.dir", "d.device", user_id)
		);
		const char* mimetype = nullptr;
		const char* file_path;
		while(qry1.assign_next_row__no_free(&mimetype, &file_path));
		if (mimetype == nullptr){
			return compsky::server::_r::not_found;
		}
		
		const size_t f_sz = os::get_file_sz(file_path);
		if (unlikely(f_sz == 0)){
			this->log("Cannot open file: ", file_path);
			return compsky::server::_r::server_error;
		}
		
		const size_t bytes_to_read = (rc == GetRangeHeaderResult::none) ? block_sz : ((to) ? (to - from) : stream_block_sz);
		const size_t bytes_read = os::read_from_file_at_offset(file_path,  this->buf + room_for_headers,  from,  bytes_to_read);
		
		const size_t end_byte = from + bytes_read;
		
		this->reset_buf_index();
		
		if (rc == GetRangeHeaderResult::none){
			// Both Firefox and Chrome send a range header for videos, neither for images
			this->asciify(
				"HTTP/1.1 200 OK\n"
				"Accept-Ranges: bytes\n"
				"Content-Type: ", mimetype, '\n',
				"Content-Length: ", f_sz, '\n',
				 '\n'
			);
		} else {
			this->asciify(
				"HTTP/1.1 206 Partial Content\n"
				"Accept-Ranges: bytes\n"
				"Content-Type: ", mimetype, '\n',
				"Content-Range: bytes ", from, '-', end_byte - 1, '/', f_sz, '\n',
				// The minus one is because the range of n bytes is from zeroth byte to the (n-1)th byte
				"Content-Length: ", bytes_read, '\n',
				'\n'
			);
		}
		
		const size_t headers_len = (uintptr_t)this->itr - (uintptr_t)this->buf;
		memcpy(this->buf + room_for_headers - headers_len,  this->buf,  headers_len);
		
		return std::string_view(this->buf + room_for_headers - headers_len,  headers_len + bytes_read);
	}
	
	FunctionSuccessness dl_or_cp_file(char(&file_path)[4096],  const char* user_headers,  const UserIDIntType user_id,  const uint64_t dir_id,  const char* const file_id,  const char* const file_name,  const char* const url,  const bool overwrite_existing,  char* mimetype,  const bool is_ytdl,  const bool is_audio_only){
		if (unlikely(file_name[0] == 0)){
			this->log("Empty file name");
			return FunctionSuccessness::server_error;
		}
		
		if (in_str(file_name, os::unix_path_sep) and (file_id==nullptr) and not is_ytdl){
			this->log("dl_or_cp_file rejected due to slash in file name: ", file_name);
			return FunctionSuccessness::server_error;
		}
		
		const StringFromSQLQuery dir_name(
			this,
			"SELECT d.full_path "
			"FROM dir d "
			"WHERE d.id=", dir_id
		); //, " AND id NOT IN " USER_DISALLOWED_DIRS(user_id));
		
		if (dir_name.value == nullptr){
			// No visible directory with the requested ID
			// MySQL results already freed
			return FunctionSuccessness::malicious_request;
		}
		
		if (not endswith(dir_name.value, os::unix_path_sep)){
			// TODO: Allow for this
			this->log("dl_or_cp_file rejected due to dir name not ending in slash: ", dir_name.value);
			return FunctionSuccessness::server_error;
		}
		
		if (not os::is_local_file_or_dir(dir_name.value)){
			return FunctionSuccessness::malicious_request;
		}
		
		// If YTDL, then file_path is the template of the path of the output file; else it is the path of the output file
		compsky::asciify::asciify(file_path, dir_name.value, (is_ytdl or file_id==nullptr)?file_name:file_id, '\0');
		
		this->log("dl_file ", (overwrite_existing)?">":"+", ' ', dir_id, ' ', url, "\n        -> ", file_path);
		
		// WARNING: Appears to freeze if the directory is not accessible (e.g. an unmounted external drive)
		// TODO: Check device is mounted
		
		if (os::is_local_file_or_dir(url)){
			if (is_ytdl)
				return FunctionSuccessness::malicious_request;
			else
				return (std::filesystem::copy_file(url, file_path)) ? FunctionSuccessness::ok : FunctionSuccessness::server_error;
		} else {
			if (is_ytdl){
				compsky::asciify::asciify(
					file_path,
					dir_name.value,
					"%(extractor)s-%(id)s.%(ext)s",
					'\0'
				);
#ifdef PYTHON
				if (this->ytdl(user_id, file_id, dir_name.value, file_path, url, is_audio_only))
#else
				const char* ytdl_args[] = {"youtube-dl", "-q", "-o", file_path, "-f", (is_audio_only)?"bestaudio":YTDL_FORMAT, url, nullptr};
				if (proc::exec(60, ytdl_args, STDERR_FILENO, file_path))
#endif
				{
					return FunctionSuccessness::server_error;
				}
				if (file_path[0] != 0){
				file_path[4096-1] = 0;
				char* const file_extension = skip_to_after<char>(file_path, "Requested formats are incompatible for merge and will be merged into ");
				if (file_extension != nullptr){
					replace_first_instance_of(file_extension, '.', '\0');
					
					compsky::asciify::asciify(
						file_path,
						dir_name.value,
						"%(extractor)s-%(id)s.", // Omit the file extension, as youtube-dl does not get the correct extension in this case when simulating (why force simulating then?!)
						'\0'
					);
				}
				
#ifndef PYTHON
				// Now run youtube-dl a second time, to paste the output filename into file_path, because it has no option to print the filename without only simulating
				const char* ytdl2_args[] = {"youtube-dl", "--get-filename", "-o", file_path, "-f", YTDL_FORMAT, url, nullptr};
				if (proc::exec(3600, ytdl2_args, STDOUT_FILENO, file_path)){
					return FunctionSuccessness::server_error;
				}
#endif
				
				if (file_extension == nullptr){
					replace_first_instance_of(file_path, '\n', '\0');
				} else {
					replace_first_instance_of(file_path, '\n', file_extension, '\0');
				}
				this->log("YTDL to: ", file_path);
				}
			} else
				return curl::dl_file(this->itr, url, file_path, overwrite_existing, mimetype) ? FunctionSuccessness::ok : FunctionSuccessness::server_error;
		}
		
		return FunctionSuccessness::ok;
	}
	
	FunctionSuccessness dl_file(char(&file_path)[4096],  const char* user_headers,  const UserIDIntType user_id,  const uint64_t dir_id,  const char* const file_id,  const char* const file_name,  const char* const url,  const bool overwrite_existing,  char* mimetype,  const bool force_remote,  const bool is_ytdl,  const bool is_audio_only){
		if (os::is_local_file_or_dir(url) and force_remote)
			return FunctionSuccessness::malicious_request;
		
		return this->dl_or_cp_file(file_path, user_headers, user_id, dir_id, file_id, file_name, url, overwrite_existing, mimetype, is_ytdl, is_audio_only);
	}
	
	template<typename Str1,  typename Str2>
	void add_t_to_db(const UserIDIntType user_id,  const Str1 parent_ids,  const Str2 tag_name){
		this->mysql_exec(
			"INSERT INTO tag "
			"(name,user)"
			"SELECT \"", _f::esc, '"', tag_name, "\",", user_id, " "
			"FROM tag "
			"WHERE NOT EXISTS"
			"(SELECT id FROM tag WHERE name=\"", _f::esc, '"', tag_name, "\")"
			"LIMIT 1"
		);
		
		ParentIDs<Str1>::
		tag_parentisation(
			this,
			user_id,
			false,
			parent_ids,
			"SELECT id FROM tag WHERE name=\"", _f::esc, '"', tag_name, "\""
		);
	}
	
	bool add_D_to_db(const UserIDIntType user_id,  const char* const tag_ids,  const size_t tag_ids_len,  const char* const url,  const size_t url_len){
		this->mysql_exec(
			"INSERT INTO device "
			"(protocol,name,user)"
			"SELECT "
				"id,"
				"\"", _f::esc, '"', _f::strlen,  url_len,  url, "\",",
				user_id, " "
			"FROM protocol "
			"WHERE LEFT(\"", _f::esc, '"', _f::strlen,  url_len,  url,  "\",LENGTH(name))=name "
			  "AND NOT EXISTS"
				"(SELECT id FROM device WHERE name=\"", _f::esc, '"', _f::strlen,  url_len,  url, "\")"
			"ORDER BY LENGTH(name) DESC "
			"LIMIT 1"
		);
		regenerate_device_json = true;
		return false;
	}
	
	std::string_view post__recursively_record_filesystem_dir(const char* s){
		GET_NUMBER(unsigned, max_depth)
		GET_COMMA_SEPARATED_INTS__NULLABLE(TRUE, tag_ids, tag_ids_len, s, ' ')
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("record_local_fs")
		SKIP_TO_BODY
		
		this->reset_buf_index();
		this->asciify(s, '\0');
		
		if (unlikely(not os::dir_exists(this->buf)))
			return compsky::server::_r::not_found;
		
		if (tag_ids != nullptr){
			// Tag the root directory the client chose
			// TODO: Guess the mimetype
			if (this->itr[-2] != os::unix_path_sep){
				this->itr[-1] = os::unix_path_sep;
				this->itr[0] = 0;
				++this->itr;
			}
			this->add_file_or_dir_to_db<0>('d', nullptr, user_id, (tag_ids==nullptr)?"0":tag_ids, (tag_ids==nullptr)?1:tag_ids_len, this->buf, this->buf_indx(), 0, false, false);
			// TODO: Use libmagic to guess mime type
		}
		
		this->recursively_record_files_infilesystem(user_id,  max_depth - 1);
		// NOTE: if max_depth is 0, this wraps around to MAX_UNSIGNED. This is deliberate - it means there is (effectively) no limit.
		
		return compsky::server::_r::post_ok;
	}
	
	void recursively_record_files_infilesystem(const UserIDIntType user_id,  const unsigned max_depth){
		os::dir_handler_typ dir = os::open_dir(this->buf);
		if (dir == nullptr)
			return;
		if (this->itr[-2]  !=  os::unix_path_sep){
			// The character before the terminating null byte
			this->itr[-1] = os::unix_path_sep;
			this->itr[0] = '\0';
			++this->itr;
		}
		const size_t dir_len = strlen(this->buf);
		os::dirent_typ e;
		while (os::get_next_item_in_dir(dir, e)){
			const char* const ename = os::get_dirent_name(e);
			
			if (os::is_not_file_or_dir_of_interest(ename))
				continue;
			
			--this->itr; // Overwrite trailing null byte
			this->asciify(ename, '\0');
			
			if (os::is_dir(e)){
				if (max_depth != 0)
					this->recursively_record_files_infilesystem(user_id,  max_depth - 1);
			} else if (e->d_type == DT_REG){
				// regular file
				this->add_file_or_dir_to_db<0>('f', nullptr, user_id, "0", 1, this->buf, this->buf_indx()-1, 0, false, false);
			}
			
			this->itr = this->buf + dir_len + 1; // Account for the terminating null byte
			*this->itr = 0;
		}
		os::close_dir(dir);
	}
	
	template<size_t i = 0,  typename... Args>
	FunctionSuccessness add_file_or_dir_to_db(Args... args){
		uint64_t dir_id;
		return this->add_file_or_dir_to_db__w_parent_dir_id<i>(dir_id, args...);
	}
	
	template<size_t i = 0>
	FunctionSuccessness add_file_or_dir_to_db__w_parent_dir_id(uint64_t& parent_dir_id,  const char which_tbl,  const char* const user_headers,  const UserIDIntType user_id,  const char* const tag_ids,  const size_t tag_ids_len,  const char* const url,  const size_t url_len,  const uint64_t dl_backup_into_dir_id,  const bool is_ytdl,  const bool is_audio_only,  const char* const mimetype = ""){
		// Add ancestor directories
		size_t offset = 0;
		parent_dir_id = 0;
		unsigned n_errors = 0;
		while(true){
			this->qry_mysql_for_next_parent_dir<i>(user_id, parent_dir_id, _f::strlen,  url_len - offset,  url + offset);
			size_t closest_parent_dir_length = 0;
			while(this->mysql_assign_next_row<i>(&parent_dir_id, &closest_parent_dir_length));
			if (unlikely(closest_parent_dir_length == 0)){
				// No such directory was found. This is probably because the user does not have permission to view an ancestor directory.
				return FunctionSuccessness::malicious_request;
			}
			offset += closest_parent_dir_length;
			size_t url_len_of_next_step = n_chars_until_char_in(url + offset, '/', '\n', '\0');
			if (url[offset + url_len_of_next_step] == '/')
				// Include the trailing slash, if it exists
				++url_len_of_next_step;
			const size_t url_len_up_until_next_step = offset + url_len_of_next_step;
			if (not in_str_not_at_end__where_end_marked_by(url + offset,  '/',  '\n', '\0')){
				/* The closest parent is also the immediate parent of the directory to add
				 * Have one final check to find the largest prefix
				 * E.g. parsing a YouTube video url "https://www.youtube.com/watch?v=dQw4w9WgXcQ":
				 *    https://         1st dir
				 *    www.youtube.com/ 2nd dir
				 *    watch?v=         3rd dir
				 * This section accounts for the final dir, if it exists
				 */
				this->qry_mysql_for_next_parent_dir<i>(user_id, parent_dir_id, _f::strlen,  url_len - offset,  url + offset);
				size_t closest_parent_dir_length = 0;
				while(this->mysql_assign_next_row<i>(&parent_dir_id, &closest_parent_dir_length));
				offset += closest_parent_dir_length;
				break;
			}
			// NOTE: The directory to add may not end in a slash. However, all its parent directories must. The following SQL will not insert the directory we wish to add, only insert all its ancestors.
			this->mysql_exec(
				"INSERT INTO dir"
				"(parent,device,user,full_path,name)"
				"SELECT "
					"id,"
					"device,",
					user_id, ","
					"\"", _f::esc, '"', _f::strlen, url_len_up_until_next_step, url, "\","
					"\"", _f::esc, '"', _f::strlen, url_len_of_next_step, url+offset, "\" "
				"FROM dir "
				"WHERE id=", parent_dir_id, " "
				// No need to check permissions, that has already been done in qry_mysql_for_next_parent_dir
				"ON DUPLICATE KEY UPDATE device=VALUES(device)"
			);
		}
		
		const std::string_view f_name_sv(url + offset,  url_len - offset);
		
		if (offset == url_len){
			// Directory that we are trying to add already existed
			// NOTE: The behaviour for attempting to add existing files/dirs is to still tag them
			
			// Move offset down, as we are currently situated after the final slash
			do {
				--offset;
			} while (url[offset] != '/');
		} else if (which_tbl == 'd'){
			// Add entry to primary table
			this->mysql_exec(
				"INSERT INTO dir "
				"(parent, device, user, full_path, name)"
				"SELECT "
					"id,",
					"device,",
					user_id, ","
					"\"", _f::esc, '"', _f::strlen, url_len, url, "\","
					"\"", _f::esc, '"',  f_name_sv, "\" "
				"FROM dir "
				"WHERE id=", parent_dir_id
				// NOTE: The user has been verified to have permission to access the parent directory.
				// Guaranteed not to be a duplicate
			);
		} else /* == 'f' */ {
			this->mysql_exec(
				"INSERT INTO file "
				"(dir, name, user, mimetype)"
				"SELECT ",
					parent_dir_id, ","
					"\"", _f::esc, '"',  f_name_sv, "\",",
					user_id, ","
					"IFNULL(mt.id,0)"
				"FROM file f "
				"JOIN dir d ON d.id=f.dir "
				"LEFT JOIN mimetype mt ON mt.name=\"", mimetype, "\" "
				"WHERE NOT EXISTS"
				"(SELECT id FROM file WHERE dir=", parent_dir_id, " AND name=\"", _f::esc, '"',  f_name_sv, "\")"
				  "AND " NOT_DISALLOWED_DIR("f.dir", "d.device", user_id)
				"LIMIT 1"
			);
			
			if (dl_backup_into_dir_id != 0){
				const char* file_name;
				const char* ext = nullptr;
				
				char mimetype[MAX_MIMETYPE_SZ + 1] = {0};
				char _buf[4096 + 1024];
				char* _itr = _buf;
				compsky::asciify::asciify(_itr, _f::strlen, url, url_len, '\0');
				get_file_name_and_ext__filename_ends_with_newline_or_null(_buf, file_name, ext);
				if (unlikely(_itr[-1] == os::unix_path_sep)){
					const size_t file_name_len = (uintptr_t)_itr - (uintptr_t)_buf - 1;
					file_name = _itr;
					compsky::asciify::asciify(_itr, _f::strlen, file_name, file_name_len, '\0');
				}
				
				const bool is_html_file  =  (ext == nullptr)  or  (ext < file_name);
				const StringFromSQLQuery file_id(this, "SELECT id FROM file WHERE dir=", parent_dir_id, " AND name=\"", _f::esc, '"', f_name_sv, "\" LIMIT 1");
				char file_path[4096];
				
				auto const successness = this->dl_file(file_path, user_headers, user_id, dl_backup_into_dir_id, file_id.value, file_name, _buf, is_html_file, mimetype, true, is_ytdl, is_audio_only);
				
				if (unlikely(successness == FunctionSuccessness::malicious_request))
					return FunctionSuccessness::malicious_request;
				
				if (likely(successness == FunctionSuccessness::server_error)){
					++n_errors;
				} else {
					if (file_path[0] != 0)
						this->insert_file_backup(nullptr, parent_dir_id, dl_backup_into_dir_id, f_name_sv, "\"", basename__accepting_trailing_slash(file_path), "\"", user_id, mimetype);
					
					if (mimetype[0]){
						this->mysql_exec(
							"UPDATE file f "
							"JOIN file_backup f2 ON f2.file=f.id "
							"SET f.mimetype=f2.mimetype "
							"WHERE f.name=\"", _f::esc, '"', f_name_sv, "\" AND f.dir=", parent_dir_id, " "
							"AND f2.name=\"", _f::esc, '"', f_name_sv, "\" "
							"AND f2.dir=", dl_backup_into_dir_id
						);
					}
				}
			}
		}
		
		// Add tags
		if (which_tbl == 'd'){
			this->add_tags_to_dirs(
				user_id,
				tag_ids, tag_ids_len,
				  "AND d.name=\"",  _f::esc, '"', _f::strlen,  url_len - offset,  url + offset, "\" "
				  "AND d.parent=", parent_dir_id, " "
				  "AND " NOT_DISALLOWED_DIR("d.id", "d.device", user_id)
			);
		} else /* if (which_tbl == 'f') */ {
			this->add_tags_to_files(
				user_id,
				std::string_view(tag_ids, tag_ids_len),
				  "AND t.id != 0 "
				  "AND f.name=\"",  _f::esc, '"', _f::strlen,  url_len - offset,  url + offset, "\" "
				  "AND f.dir=", parent_dir_id, " "
				  "AND " NOT_DISALLOWED_DIR("f.dir", "d.device", user_id)
			);
		}
		
		this->log("n_errors: ", n_errors);
		
		return (n_errors) ? FunctionSuccessness::server_error : FunctionSuccessness::ok;
	}
	
	template<typename T,  typename... Args>
	T get_last_row_from_qry(Args... args){
		SQLQuery qry(this, db_infos.at(0), args...);
		T t = 0;
		qry.assign_next_row(&t);
		return t;
	}
	
	std::string_view post__add_boxes(const char* s){
		GET_NUMBER_NONZERO(uint64_t,file_id)
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("add_boxes")
		SKIP_TO_BODY
		--s;
		
		// TODO: Perform a permissions check on file_id
		const uint64_t below_new_box_ids = this->get_last_row_from_qry<uint64_t>("SELECT IFNULL(MAX(id),0) FROM box WHERE file=", file_id);
		
		// This function returns an array of the new box IDs
		this->begin_json_response();
		this->asciify('[');
		
		do {
			++s; // Skip trailing newline
			GET_NUMBER(uint64_t,box_id) // It is 0 for an entirely new box
			GET_NUMBER(uint64_t,frame_n)
			GET_FLOAT(double,x)
			GET_FLOAT(double,y)
			GET_FLOAT(double,w)
			GET_FLOAT(double,h)
			GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, '\n')
			
			uint64_t box_id_final = box_id;
			
			if (box_id == 0){
				this->mysql_exec(
					"INSERT INTO box"
					"(file,frame_n,x,y,w,h,user)"
					"VALUES(",
						file_id, ',',
						frame_n, ',',
						x, 3, ',',
						y, 3, ',',
						w, 3, ',',
						h, 3, ',',
						user_id,
					")"
					"ON DUPLICATE KEY UPDATE id=id" // WARNING: In this case, tags may not be applied if the old user is not equal to the new user
				);
				// NOTE: To add tags without knowing the box IDs, and knowing that floats are not stored exactly, we must find the box whose coordinates most closely match the given coordinates
				
				box_id_final = this->get_last_row_from_qry<uint64_t>(
					"SELECT id "
					"FROM box "
					"WHERE file=", file_id, " "
						"AND user=", user_id, " "
					"ORDER BY "
						"POWER(x-",x,3,",2)+"
						"POWER(y-",y,3,",2)+"
						"POWER(w-",y,3,",2)+"
						"POWER(h-",h,3,",2) "
					"LIMIT 1"
				);
				
				this->asciify(box_id_final, ',');
			} else {
				this->mysql_exec(
					"UPDATE box "
					"SET "
						"frame_n=", frame_n, ","
						"x=", x, 3, ","
						"y=", y, 3, ","
						"w=", w, 3, ","
						"h=", h, 3, " "
					"WHERE id=", box_id, " "
					  "AND file=", file_id // Security/sensibleness feature
					// TODO: Record user ID somehow (per update? or change the user of the box?)
				);
			}
			
			this->mysql_exec(
				"INSERT INTO box2tag"
				"(box,tag,user)"
				"SELECT ",
					box_id_final, ","
					"t.id,",
					user_id, " "
				"FROM tag t "
				"WHERE t.id IN(", _f::strlen, tag_ids, tag_ids_len, ")"
				  "AND " NOT_DISALLOWED_TAG("t.id", user_id)
				"ON DUPLICATE KEY UPDATE box=box"
			);
			
			if (s[1] == 0)
				// NOTE: s[0] != 0 is guaranteed by GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL
				break;
		} while(true);
		
		if (this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify(']');
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view add_to_tbl(const char tbl,  const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, '/')
		// NOTE: A tag_ids of "0" should be allowed, at least for adding directories.
		++s; // Skip trailing slash
		GET_NUMBER(uint64_t,dir_id)
		GET_NUMBER(bool,is_ytdl)
		GET_NUMBER(bool,is_audio_only)
		
		const char* const user_headers = s;
		
		GET_USER_ID
		SKIP_TO_BODY
		--s;
		
		do {
			++s; // Skip trailing newline
			const char* const url = s;
			while ((*s != 0) and (*s != '\n'))
				++s;
			const size_t url_len = (uintptr_t)s - (uintptr_t)url;
			if (url_len == 0)
				return compsky::server::_r::not_found;
			switch(tbl){
				case 'f':
				case 'd':
					switch(this->add_file_or_dir_to_db(tbl, user_headers, user_id, tag_ids, tag_ids_len, url, url_len, dir_id, is_ytdl, is_audio_only)){
						case FunctionSuccessness::server_error:
							return compsky::server::_r::server_error;
						case FunctionSuccessness::malicious_request:
							return compsky::server::_r::not_found;
					}
					break;
				case 'D':
					if (unlikely(this->add_D_to_db(user_id, tag_ids, tag_ids_len, url, url_len)))
						return compsky::server::_r::not_found;
					break;
				case 't':
					this->add_t_to_db(user_id, std::string_view(tag_ids, tag_ids_len), std::string_view(url, url_len));
					break;
			}
			if (*s == 0)
				return compsky::server::_r::post_ok;
		} while(true);
	}
	
	std::string_view add_era(const char* s){
		GET_NUMBER_NONZERO(uint64_t, file_id)
		
		const float era_start = a2f<float>(&s);
		if(*s != '-')
			return compsky::server::_r::not_found;
		++s; // Skip dash
		const float era_end   = a2f<float>(&s);
		if(*s != '/')
			return compsky::server::_r::not_found;
		++s; // Skip slash
		
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, ' ')
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("add_eras")
		
		// NOTE: The permission "link_tags" is deliberately not needed, as I think it can be useful to allow the creation of new eras with tags, while restricting more heavily the editing of existing tags.
		
		this->mysql_exec(
			"INSERT INTO era"
			"(file,start,end,user)"
			"SELECT "
				"f.id,",
				era_start, 5, ',',
				era_end, 5, ',',
				user_id, " "
			"FROM file f "
			"JOIN dir d ON d.id=f.dir "
			"WHERE f.id=", file_id, " "
			  "AND " NOT_DISALLOWED_FILE("f.id", "f.dir", "d.device", user_id)
		);
		
		this->add_tag_to_era(
			user_id,
			tag_ids, tag_ids_len,
			  "AND e.file=",  file_id, " "
			  "AND e.start=", era_start, 5, " "
			  "AND e.end=",   era_end, 5
		);
		
		return compsky::server::_r::post_ok;
	}
	
	template<typename... Args>
	void add_tag_to_era(const UserIDIntType user_id,  const char* const tag_ids,  const size_t tag_ids_len,  Args... where_args){
		this->mysql_exec(
			"INSERT INTO era2tag"
			"(era, tag)"
			"SELECT "
				"e.id,"
				"t.id "
			"FROM tag t "
			"JOIN era e "
			"JOIN file f ON f.id=e.file "
			"JOIN dir d ON d.id=f.dir "
			"WHERE t.id IN (", _f::strlen, tag_ids,  tag_ids_len,  ")"
			  "AND " NOT_DISALLOWED_TAG("t.id", user_id)
			  "AND " NOT_DISALLOWED_ERA("e.id", "e.file", "f.dir", "d.device", user_id),
			  where_args..., " "
			"ON DUPLICATE KEY UPDATE era=era"
		);
	}
	
	std::string_view update_tag_thumbnail(const char* s){
		GET_NUMBER_NONZERO(uint64_t, tag_id)
		
		const char* const url = s;
		const size_t url_length = count_until(url, ' ');
		
		if(*s == 0)
			return compsky::server::_r::not_found;
		
		GET_USER_ID
		BLACKLIST_GUEST
		
		this->mysql_exec(
			"UPDATE tag "
			"SET thumbnail=\"", _f::esc, '"', _f::strlen, url_length, url, "\" "
			"WHERE id=", tag_id
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view archive_reddit_post(const char* s){
		// WARNING: No permissions check
		
		const char* const permalink = s;
		while((*s != 0) and (*s != ' '))
			++s;
		if(*s == 0)
			return compsky::server::_r::not_found;
		const size_t permalink_length = (uintptr_t)s - (uintptr_t)permalink;
		
		this->asciify(_f::strlen, permalink, permalink_length, '\0');
		
#ifdef PYTHON__NOTIMPLEMENTEDYET
		const bool failed = python::archive_reddit_post(this->buf);
#else
		const char* args[] = {"record-reddit-post", this->buf, nullptr};
		const bool failed = proc::exec(60,  args,  STDOUT_FILENO,  nullptr,  0);
#endif
		if (unlikely(failed))
			return compsky::server::_r::server_error;
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__set_dir_attr(const char* s,  const char* const attr){
		GET_NUMBER_NONZERO(uint64_t, dir_id)
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("edit_names")
		SKIP_TO_BODY
		
		this->mysql_exec(
			"UPDATE dir d "
			"SET d.", attr, "=\"", _f::esc, '"', s, "\" "
			"WHERE d.id=", dir_id, " "
			  "AND " NOT_DISALLOWED_DIR("d.id", "d.device", user_id)
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__set_tag_attr(const char* s,  const char* const attr){
		GET_NUMBER_NONZERO(uint64_t, tag_id)
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("edit_names")
		SKIP_TO_BODY
		
		this->mysql_exec(
			"UPDATE tag t "
			"SET t.", attr, "=\"", _f::esc, '"', s, "\" "
			"WHERE t.id=", tag_id, " "
			  "AND " NOT_DISALLOWED_TAG("t.id", user_id)
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__set_file_attr(const char* s,  const char* const attr){
		GET_NUMBER_NONZERO(uint64_t, f_id)
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("edit_names")
		SKIP_TO_BODY
		
		this->mysql_exec(
			"UPDATE file f "
			"JOIN dir d ON d.id=f.dir "
			"SET f.", attr, "=\"", _f::esc, '"', s, "\" "
			"WHERE f.id=", f_id, " "
			  "AND " NOT_DISALLOWED_FILE("f.id", "f.dir", "d.device", user_id)
		);
		
		return compsky::server::_r::post_ok;
	}
	
	template<typename... Args>
	void merge_files(const UserIDIntType user_id,  const uint64_t orig_f_id,  Args... dupl_f_ids_args){
		this->mysql_exec("DELETE FROM file2tag WHERE file IN (", dupl_f_ids_args..., ") AND tag IN (SELECT * FROM (SELECT tag FROM file2tag WHERE file=", orig_f_id, ") AS t)");
		this->mysql_exec("INSERT INTO file2tag (file,tag,user) SELECT ", orig_f_id, ", tag, user FROM file2tag f2t WHERE file IN (", dupl_f_ids_args..., ") ON DUPLICATE KEY update file2tag.file=file2tag.file");
		this->mysql_exec("DELETE FROM file2tag WHERE file IN (", dupl_f_ids_args..., ")");
		
		this->mysql_exec("DELETE FROM file2thumbnail WHERE file IN (", dupl_f_ids_args..., ")");
		
		this->mysql_exec("DELETE FROM file_backup WHERE file IN (", dupl_f_ids_args..., ") AND dir IN (SELECT * FROM (SELECT dir FROM file_backup WHERE file=", orig_f_id, ") AS t)");
		this->mysql_exec("UPDATE file_backup SET file=", orig_f_id, " WHERE file IN (", dupl_f_ids_args..., ")");
		
		this->mysql_exec("INSERT INTO file_backup (file,dir,name,mimetype,user) SELECT ", orig_f_id, ", f.dir, f.name, f.mimetype, ", user_id, " FROM file f WHERE f.id IN (", dupl_f_ids_args..., ") ON DUPLICATE KEY UPDATE file=file"); // WARNING: I think if there's a duplicate key, something has gone wrong previously.
		this->mysql_exec("DELETE FROM file2post WHERE post IN (SELECT * FROM (SELECT post FROM file2post WHERE file=", orig_f_id, ") AS t) AND file IN(", dupl_f_ids_args..., ")");
		this->mysql_exec("UPDATE file2post SET file=", orig_f_id, " WHERE file IN(", dupl_f_ids_args..., ")");
		for (const char* tbl_name : tables_referencing_file_id){
			if (not this->get_last_row_from_qry<bool>("SELECT 1 FROM ", tbl_name, " WHERE file=", orig_f_id))
				this->mysql_exec("UPDATE ", tbl_name, " SET file=", orig_f_id, " WHERE file IN(", dupl_f_ids_args..., ") LIMIT 1");
			this->mysql_exec("DELETE FROM ", tbl_name, " f WHERE f.file IN(", dupl_f_ids_args..., ")");
		}
		this->mysql_exec("DELETE FROM file WHERE id IN (", dupl_f_ids_args..., ")");
	}
	
	std::string_view post__merge_files(const char* s){
		GET_NUMBER_NONZERO(uint64_t, orig_f_id)
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, dupl_f_ids, dupl_f_ids_len, s, ' ')
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("merge_files")
		
		this->merge_files(user_id, orig_f_id, _f::strlen, dupl_f_ids, dupl_f_ids_len);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__backup_file(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, file_ids, file_ids_len, s, '/')
		++s; // Skip slash
		GET_NUMBER_NONZERO(uint64_t, dir_id)
		GET_NUMBER(bool, is_ytdl)
		GET_NUMBER(bool, is_audio_only)
		
		const char* const url = s; // An URL which (if supplied) is used instead of the original file URL
		const size_t url_length = count_until(url, ' ');
		
		const char* const user_headers = s;
		
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("backup_files")
		
		// TODO: Hide this option for guests in the UI, and BLACKLIST_GUESTS in this function
		
		SQLQuery qry(this, db_infos.at(0), "SELECT f.id, d.full_path, f.name FROM file f JOIN dir d ON d.id=f.dir WHERE f.id IN(", _f::strlen, file_ids, file_ids_len, ")");
		char orig_file_path[4096];
		const char* file_id_str;
		const char* orig_dir_name;
		const char* file_name;
		while(qry.assign_next_row(&file_id_str, &orig_dir_name, &file_name)){
			if ((url_length != 0) and not is_ytdl)
				compsky::asciify::asciify(orig_file_path, _f::strlen, url, url_length, '\0');
			else
				compsky::asciify::asciify(orig_file_path, orig_dir_name, file_name, '\0');
			
			char mimetype[100] = {0};
			char file_path[4096];
			const auto rc = this->dl_or_cp_file(file_path, user_headers, user_id, dir_id, file_id_str, file_name, orig_file_path, false, mimetype, is_ytdl, is_audio_only);
			
			if (rc != FunctionSuccessness::ok)
				return (rc == FunctionSuccessness::malicious_request) ? compsky::server::_r::not_found : compsky::server::_r::server_error;
			
			if (file_path[0] != 0)
				this->insert_file_backup(file_id_str, 0, dir_id, "!!!/NOSUCHNAME/!!!", "SUBSTR(\"", file_path, "\",LENGTH(d.full_path)+1)", user_id, mimetype);
			// WARNING: The above will crash if there is no such extension in ext2mimetype
			// This is deliberate, to tell me to add it to the DB.
		}
		
		return compsky::server::_r::post_ok;
	}
	
	template<typename StrType>
	void insert_file_backup(
		const char* const file_id,
		const uint64_t file_dir,
		const uint64_t backup_dir,
		const StrType file_name,
		const char* const file_backup_name_pre,
		const char* const file_backup_name,
		const char* const file_backup_name_post,
		const UserIDIntType user_id,
		char* const mimetype
	){
		const bool is_mimetype_set = (mimetype[0]);
		this->mysql_exec(
			"INSERT INTO file_backup "
			"(file,dir,name,mimetype,user)"
			"SELECT ",
				(file_id)?file_id:"f.id", ',',
				backup_dir, ',',
				file_backup_name_pre, _f::esc, '"', file_backup_name, file_backup_name_post, ","
				"IFNULL(mt.id,0),",
				user_id, " "
			"FROM file f "
			"JOIN dir d ON d.id=", backup_dir, " "
			"LEFT JOIN ",
				(is_mimetype_set)?"mimetype":"ext2mimetype",
				" mt ON mt.name=SUBSTRING_INDEX(\"",
				_f::esc, '"',
				(is_mimetype_set)?mimetype:file_name, // TODO: Escape mimetype properly
				(is_mimetype_set)?"\",';',1":"\",'.',-1", ") "
			"WHERE f.id=", (file_id)?file_id:"0", " "
			"OR("
				"f.name=\"", _f::esc, '"', file_name, "\" "
				"AND "
				"f.dir=", file_dir,
			")"
			"ON DUPLICATE KEY UPDATE file_backup.mimetype=VALUES(mimetype)"
		);
	}
	
	void tag_antiparentisation(const UserIDIntType user_id,  const char* const child_ids,  const char* const tag_ids,  const size_t child_ids_len,  const size_t tag_ids_len){
		this->mysql_exec(
			"DELETE t2p "
			"FROM tag2parent t2p "
			"JOIN tag t ON t.id=t2p.id "
			"JOIN tag p ON p.id=t2p.parent "
			"WHERE t.id IN (", _f::strlen, child_ids, child_ids_len, ")"
			  "AND p.id IN (", _f::strlen, tag_ids,   tag_ids_len,   ")"
			  "AND " NOT_DISALLOWED_TAG("t.id", user_id)
			  // "AND p.id NOT IN" USER_DISALLOWED_TAGS(user_id) // Unnecessary
		);
		
		// TODO: Descendant tags etc
	}
	
	template<typename... ParentIDsArgs>
	struct ParentIDs {
		template<typename... ChildIDsArgs>
		static
		void tag_parentisation(
			TagemResponseHandler* thees,
			const UserIDIntType user_id,
			const bool update_descendants,
			ParentIDsArgs... parent_ids_args,
			ChildIDsArgs... child_ids_args
		){
			thees->mysql_exec(
				"INSERT INTO tag2parent (id, parent, user) "
				"SELECT t.id, p.id,", user_id, " "
				"FROM tag t "
				"JOIN tag p "
				"WHERE t.id IN (", child_ids_args..., ")"
				  "AND p.id IN (", parent_ids_args...,   ")"
				  "AND " NOT_DISALLOWED_TAG("t.id OR _t2pt.id=p.id", user_id)
				"ON DUPLICATE KEY UPDATE parent=parent"
			);
			
			thees->mysql_exec(
				"INSERT INTO tag2parent_tree (id, parent, depth)"
				"SELECT * "
				"FROM("
					"SELECT id AS id, id AS parent, 0 AS depth "
					"FROM tag "
					"WHERE id IN(", child_ids_args..., ")"
					"UNION "
					"SELECT t.id, t2pt.parent, t2pt.depth+1 "
					"FROM tag t "
					"JOIN tag2parent_tree t2pt "
					"WHERE t.id IN(", child_ids_args..., ")"
					  "AND t2pt.parent IN (", parent_ids_args...,   ")"
					  "AND " NOT_DISALLOWED_TAG("t.id OR _t2pt.id=t2pt.id", user_id)
				")A "
				"ON DUPLICATE KEY UPDATE depth=LEAST(tag2parent_tree.depth, A.depth)"
			);
			
			if (update_descendants){
				thees->mysql_exec(
					"INSERT INTO tag2parent_tree (id, parent, depth) "
					"SELECT t2pt.id, t2pt2.parent, t2pt.depth+1 "
					"FROM tag2parent_tree t2pt "
					"JOIN tag2parent_tree t2pt2 ON t2pt2.id=t2pt.parent "
					"WHERE t2pt.id IN (", parent_ids_args..., ")"
					  "AND " NOT_DISALLOWED_TAG("t2pt.id", user_id)
					"ON DUPLICATE KEY UPDATE depth=LEAST(tag2parent_tree.depth, t2pt.depth+1)"
				);
			}
		}
	};
	
	std::string_view post__add_parents_to_tags(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, child_ids, child_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, parent_ids, parent_ids_len, s, ' ')
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("edit_tags")
		
		ParentIDs<std::string_view>::
		tag_parentisation(
			this,
			user_id,
			true,
			std::string_view(parent_ids, parent_ids_len),
			std::string_view(child_ids,  child_ids_len)
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__rm_parents_from_tags(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, parent_ids, parent_ids_len, s, ' ')
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("edit_tags")
		
		this->tag_antiparentisation(user_id, tag_ids, parent_ids, tag_ids_len, parent_ids_len);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__rm_children_from_tags(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, child_ids, child_ids_len, s, ' ')
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("edit_tags")
		
		this->tag_antiparentisation(user_id, child_ids, tag_ids, child_ids_len, tag_ids_len);
		
		return compsky::server::_r::post_ok;
	}
	
	template<typename... Args>
	void add_tags_to_dirs(const UserIDIntType user_id,  const char* const tag_ids,  const size_t tag_ids_len,  Args... where_args){
		this->mysql_exec(
			"INSERT INTO dir2tag"
			"(tag, dir, user)"
			"SELECT t.id,d.id,", user_id, " "
			"FROM tag t "
			"JOIN dir d "
			"WHERE t.id IN (", _f::strlen, tag_ids,  tag_ids_len,  ")"
			  "AND " NOT_DISALLOWED_TAG("t.id", user_id),
			  where_args..., " "
			"ON DUPLICATE KEY UPDATE dir=dir"
		);
	}
	
	template<typename T,  typename... Args>
	void add_tags_to_files(const UserIDIntType user_id,  const T tag_ids,  Args... where_args){
		this->mysql_exec(
			"INSERT INTO file2tag"
			"(tag, file, user)"
			"SELECT t.id,f.id,", user_id, " "
			"FROM tag t "
			"JOIN file f "
			"JOIN dir d ON d.id=f.dir "
			"WHERE t.id IN (", tag_ids,  ")"
			"AND " NOT_DISALLOWED_TAG("t.id", user_id),
			where_args..., " "
			"ON DUPLICATE KEY UPDATE file=file"
		);
	}
	
	template<typename... Args>
	void rm_tags_from_files(const UserIDIntType user_id,  const char* const tag_ids,  const size_t tag_ids_len,  Args... file_ids_args){
		this->mysql_exec(
			"DELETE f2t "
			"FROM file2tag f2t "
			"JOIN tag t ON t.id=f2t.tag "
			"WHERE t.id IN (", _f::strlen, tag_ids,  tag_ids_len,  ")"
			  "AND f2t.file IN (", file_ids_args..., ")"
			  "AND " NOT_DISALLOWED_TAG("t.id", user_id)
		);
	}
	
	std::string_view post__rm_tags_from_files(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, file_ids, file_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, ' ')
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("unlink_tags")
		
		const bool authorised = this->get_last_row_from_qry<bool>(
			"SELECT 1 "
			"FROM file f "
			"JOIN dir d ON d.id=f.dir "
			"WHERE f.id IN (", _f::strlen, file_ids, file_ids_len, ")"
			  "AND " NOT_DISALLOWED_FILE("f.id", "f.dir", "d.device", user_id)
		);
		if (unlikely(not authorised))
			return compsky::server::_r::not_found;
		
		this->rm_tags_from_files(user_id, tag_ids, tag_ids_len, _f::strlen, file_ids, file_ids_len);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__add_tags_to_eras(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, ids, ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, ' ')
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("link_tags")
		
		this->add_tag_to_era(user_id, tag_ids, tag_ids_len, "AND e.id IN(", _f::strlen, ids, ids_len, ")");
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__add_tags_to_dirs(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, dir_ids, dir_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, ' ')
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("link_tags")
		
		this->add_tags_to_dirs(
			user_id,
			tag_ids, tag_ids_len,
			"AND d.id IN(", _f::strlen, dir_ids, dir_ids_len, ")"
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__add_tag_to_file(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, file_ids, file_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, ' ')
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("link_tags")
		
		this->add_tags_to_files(
			user_id,
			std::string_view(tag_ids, tag_ids_len),
			"AND f.id IN(", _f::strlen, file_ids, file_ids_len, ")"
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__add_var_to_file(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, file_ids, file_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_NUMBER(uint64_t, value)
		GET_FILE2_VAR_NAME(s)
		
		const UserIDIntType user_id = user->id;
		GREYLIST_GUEST
		
		this->mysql_exec(
			"INSERT INTO file2", _f::strlen, file2_var_name, file2_var_name_len, " "
			"(file,x)"
			"SELECT f.id,", value, " "
			"FROM file f "
			"JOIN dir d ON d.id=f.dir "
			"WHERE f.id IN(", _f::strlen, file_ids, file_ids_len, ")"
			  "AND " NOT_DISALLOWED_FILE("f.id", "f.dir", "d.device", user_id)
			"ON DUPLICATE KEY UPDATE x=VALUES(x)"
		);
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view post__edit_tag_cmnt(const char* s){
		GET_NUMBER_NONZERO(uint64_t,tag_id)
		
		this->log("Edit tag cmnt:", tag_id, ": ", s);
		
		return compsky::server::_r::post_ok;
	}
	
	void md5_hash(uint8_t* const hash,  const char* const string,  const size_t string_len){
		MD5_CTX md5_ctx;
		MD5_Init(&md5_ctx);
		MD5_Update(&md5_ctx, string, string_len);
		MD5_Final(hash, &md5_ctx);
	}
	
	void md5_hash(uint8_t* const hash,  const char* const string){
		this->md5_hash(hash, string, strlen(string));
	}
	
	void md5_hash_local_file(uint8_t* const hash,  const char* const dir,  const char* const file){
		char file_path[4096];
		compsky::asciify::asciify(file_path, "file://", _f::esc_spaces_and_non_ascii, dir, _f::esc_spaces_and_non_ascii, file, '\0');
		this->md5_hash(hash, file_path);
	}
	
	std::string_view update_video_metadatas(const char* s){
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("exec_safe_tasks")
		
		this->mysql_query<0>(
			"SELECT "
				"IFNULL(f.size,0),"
				"f.mimetype,"
				"f.id,"
				"CONCAT(d.full_path,REGEXP_REPLACE(f.name, BINARY '[.][a-z34]{3,4}$', BINARY '')),"
				"CONCAT(d2.full_path,f2.name)"
			"FROM file f "
			"JOIN dir d ON d.id=f.dir "
			"LEFT JOIN file_backup f2 ON f2.file=f.id AND d.full_path NOT LIKE '/%' "
			"LEFT JOIN dir d2 ON d2.id=f2.dir AND d2.full_path LIKE '/%' "
			"WHERE f.status=0 "
			  "AND f.likes IS NULL " // TODO: Allow field to be specified? Some websites do not provide all fields. Might wish to simply get information for files rather than updating already satisfactory files.
			"ORDER BY RAND()" // NOTE: It is best to randomise the order in order to minimise the number of consecutive calls to the same website.
		);
		
		size_t file_size;
		unsigned mimetype;
		uint64_t fid;
		const char* url;
		const char* backup_url;
		unsigned i = 0;
		while(this->mysql_assign_next_row<0>(&file_size, &mimetype, &fid, &url, &backup_url)){
			if ((mimetype == 0) and not os::is_local_file_or_dir(url) and python::is_valid_ytdl_url(url) and (unlikely(this->get_remote_video_metadata(user_id, fid, url)))){
				this->mysql_exec(
					"UPDATE file "
					"SET status=1 "
					"WHERE id=", fid
				);
			}
			if ((file_size == 0) and (os::is_local_file_or_dir(url) or (backup_url != nullptr))){
				const size_t sz = os::get_file_sz(backup_url ? backup_url : url);
				if (sz != 0){
					this->mysql_exec(
						"UPDATE file "
						"SET size=IFNULL(file.size,", sz, ")"
						"WHERE id=", fid
					);
				}
			}
			if (++i & 64){
				// Every 64 iterations, check for 'kill signal'
				if (unlikely(atomic_signal::stop_updating_video_metadatas.load(std::memory_order_acquire))){
					mysql_free_result(this->res[0]);
				}
			}
		}
		
		return compsky::server::_r::post_ok;
	}
	
	std::string_view stop_update_video_metadatas(const char* s){
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("exec_safe_tasks")
		
		atomic_signal::stop_updating_video_metadatas.store(true, std::memory_order_acquire);
		return compsky::server::_r::post_ok;
	}
	
	std::string_view generate_thumbnails(const char* s){
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("exec_safe_tasks")
		
		char* const thumbnail_filepath = this->itr;
		this->asciify(CACHE_DIR);
		char* const thumbnail_filename = this->itr;
		for (const bool using_backup : {false, true}){
		for (const uint64_t device : connected_local_devices){
			this->mysql_query<0>(
				"SELECT ",
					(using_backup)?"f2.file":"f.id", ","
					"d.full_path,",
					(using_backup)?"f2":"f", ".name,"
					"(mt.name REGEXP BINARY '^video/')"
				"FROM file f ",
				(using_backup)?"JOIN file_backup f2 ON f2.file=f.id ":"",
				"JOIN dir d ON d.id=f", (using_backup)?"2":"", ".dir "
				"JOIN mimetype mt ON mt.id=f", (using_backup)?"2":"", ".mimetype "
				"WHERE device=", device, " "
				  "AND mt.name REGEXP BINARY '^(image|video)/' "
				  "AND f.md5_of_path IS NULL"
			);
			const char* fid;
			const char* dir;
			const char* file;
			bool is_video;
			while(this->mysql_assign_next_row<0>(&fid, &dir, &file, &is_video)){
				std::array<uint8_t, 16> hash;
				this->md5_hash_local_file(hash.data(), dir, file);
				char* thumbnail_filename_itr = thumbnail_filename;
				compsky::asciify::asciify(thumbnail_filename_itr, _f::lower_case, _f::hex, hash, ".png", '\0');
				
				char* const _buf = thumbnail_filename_itr;
				compsky::asciify::asciify(thumbnail_filename_itr,
					"UPDATE file "
					"SET md5_of_path=\""
				);
				for (auto i = 0;  i < 16;  ++i){
					char c = static_cast<char>(hash.at(i));
					if ((c == 0) or (c == '\\') or (c == '"'))
						compsky::asciify::asciify(thumbnail_filename_itr, '\\');
					if (c == 0)
						c = '0';
					compsky::asciify::asciify(thumbnail_filename_itr, c);
				}
				compsky::asciify::asciify(thumbnail_filename_itr, "\" WHERE id=", fid);
				
				if (os::file_exists(thumbnail_filepath))
					goto update_md5hash_then_continue;
				
				char file_path[4096];
				compsky::asciify::asciify(file_path, dir, file, '\0');
				
				if (is_video){
					try {
						generate_thumbnail(file_path, thumbnail_filepath);
						this->log("Generated video thumbnail: ", file_path, " -> ", thumbnail_filepath);
					} catch(std::exception& e){
						this->log("While generating thumbnail\n\tFile: ", file_path, "\n\tError:",  e.what());
					}
					goto update_md5hash_then_continue;
				}
				
				{
				cimg_library::CImg<unsigned char> img; // WARNING: Might get errors with other kinds of colour spaces
				try {
					img.load(file_path);
				} catch(std::exception& e){
					this->log("While generating thumbnail\n\tFile: ", file_path, "\n\tError: ",  e.what());
					goto update_md5hash_then_continue;
				}
				const unsigned int w = (img.width() >= img.height())
					? 256
					: ((img.width() * 256) / img.height())
				;
				const unsigned int h = (img.width() >= img.height())
					? ((img.height() * 256) / img.width())
					: 256
				;
				img.resize(w, h);
				img.save(thumbnail_filepath);
				}
				this->log("Generated image thumbnail: ", thumbnail_filepath);
				
				update_md5hash_then_continue:
				this->mysql_exec_buf(_buf,  (uintptr_t)thumbnail_filename_itr - (uintptr_t)_buf);
			}
		}
		}
		
		return compsky::server::_r::post_ok; // NOTE: this is likely to have timed out already on the client's side
	}
	
	template<typename AuthorType,  typename FileIDType>
	void add_new_uploader_tag_to_file(const UserIDIntType user_id,  const AuthorType author,  const char* const author_title,  const FileIDType file_id){
		char* _itr = this->itr;
		
		this->asciify(author_title, author, '\0');
		this->add_t_to_db(user_id, UPLOADER_TAG_ID, _itr);
		
		const uint64_t tag_id = this->get_last_row_from_qry<uint64_t>(
			"SELECT id "
			"FROM tag "
			"WHERE name=\"", author_title, _f::esc, '"', author, "\" "
			"LIMIT 1"
		);
		
		this->itr = _itr;
		
		this->add_tags_to_files(
			user_id,
			tag_id,
			"AND f.id=", file_id, " "
		);
	}
	
#ifdef PYTHON
	template<typename FileIDType>
	bool process_remote_video_metadata(const UserIDIntType user_id,  const FileIDType file_id,  const char* const json){
		using namespace python;
		using namespace rapidjson;
		
		Document d;
		
		if (d.Parse(json).HasParseError())
			return true;
		
		const unsigned w = get_int(d, "width");
		const unsigned h = get_int(d, "height");
		const float duration = get_flt(d, "duration");
		const float fps = get_flt(d, "fps");
		const unsigned views = get_int(d, "view_count");
		const unsigned likes = get_int(d, "like_count");
		const unsigned dislikes = get_int(d, "dislike_count");
		unsigned t_origin = (unsigned)get_flt(d, "timestamp");
		const size_t size = get_int(d, "size");
		
		const char* const title = get_str(d, "title", "");
		const char* const thumbnail = get_str(d, "thumbnail");
		const char* const descr = get_str(d, "description", "");
		const char* const dt = get_str(d, "upload_date");
		const char* const uploader = get_str(d, "uploader");
		
		if (uploader != nullptr)
			this->add_new_uploader_tag_to_file(user_id, uploader, "Uploader: ", file_id);
		
		if ((thumbnail != nullptr) and (thumbnail[0] == 'h')){
			this->mysql_exec(
				"INSERT INTO file2thumbnail"
				"(file,x)"
				"VALUES(",
					file_id, ","
					"\"", _f::esc, '"', thumbnail, "\""
				")"
				"ON DUPLICATE KEY UPDATE file=file"
			);
		}
		
		this->log("t_origin == ", t_origin);
		
		if ((t_origin == 0) and (dt != nullptr)){
			struct tm time;
			strptime(dt, "%Y%m%d", &time);
			t_origin = mktime(&time);
		}
		
		this->mysql_exec(
			"UPDATE file "
			"SET "
				"status=2,"
				"w=IFNULL(file.w,", w, "),"
				"h=IFNULL(file.h,", h, "),"
				"fps=IFNULL(file.fps,", fps, 3, "),"
				"duration=IFNULL(file.duration,", duration, 3, "),"
				"views=IFNULL(file.views,", views, "),"
				"likes=IFNULL(file.likes,", likes, "),"
				"dislikes=IFNULL(file.dislikes,", dislikes, "),"
				"size=IFNULL(file.size,", size, "),"
				"title=IFNULL(file.title,LEFT(\"", _f::esc, '"', title, "\",100)),"
				"description=IFNULL(file.description,LEFT(\"", _f::esc, '"', descr, "\",1000)),"
				"t_origin=IF(IFNULL(file.t_origin,0),file.t_origin,", t_origin, ")"
			"WHERE id=", file_id
		);
		
		return false;
	}
	
	
	template<typename FileIDType>
	bool ytdl(const UserIDIntType user_id,  const FileIDType file_id,  const char* dest_dir,  char* const out_fmt_as_input__resulting_fp_as_output,  const char* const url,  const bool is_audio_only){
#ifdef ENABLE_SPREXER
		std::string_view author{};
		const char* author_title;
		if (not info_extractor::record_info(file_id, dest_dir, out_fmt_as_input__resulting_fp_as_output, this->itr, url, author, author_title)){
			if (not author.empty())
				this->add_new_uploader_tag_to_file(user_id, author, author_title, file_id);
			return false;
		}
#endif
		
		using namespace python;
		
		bool failed;
		GILLock gillock();
		{ // Ensures the GILLock destructor is called after all PyObj destructors
		
		tagem_module::file_path = nullptr;
		
		PyDict<9> opts(
			"max_downloads", PyLong_FromLong(1), // Do not get trapped into scraping multiple pages based on the url, for instance the url being a search page
			"quiet", Py_True,
			"ffmpeg_location", tagem_module::ffmpeg_location,
			"forcefilename", Py_True,
			"outtmpl", PyUnicode_FromString(out_fmt_as_input__resulting_fp_as_output),
			"noprogress", Py_True,
			"dump_single_json", Py_True,
			"forcejson", Py_True,
			"format", PyUnicode_FromString((is_audio_only) ? "bestaudio" : YTDL_FORMAT)
		);
		Py_INCREF(tagem_module::ffmpeg_location);
		Py_INCREF(Py_True);
		Py_INCREF(Py_True);
		Py_INCREF(Py_True);
		Py_INCREF(Py_True);
		Py_INCREF(Py_True);
		PyObj ytdl_instantiation(ytdl_obj.call(opts.obj));
		// Override to_stdout, so that the file path is written to a buffer instead of stdout
		ytdl_instantiation.set_attr("to_stdout", tagem_module::to_stdout);
		
		/*PyObject_SetAttrString(tagem_module::modul, "ytdl_instantiation", ytdl_instantiation);
		PyRun_String("tagem.ytdl_instantiation.to_stdout = tagem.to_stdout", Py_single_input, nullptr, nullptr);
		// The C API call acts differently - the first argument is NOT a pointer to the 'self' object, but remains a pointer to the module object*/
		
		try {
			ytdl_instantiation.call_fn_void("download", PyObj(Py_BuildValue("[s]", url)).obj);
		} catch(const std::runtime_error& e){
			// Python exception, almost certainly an invalid URL or deleted file
			// Most likely occurs after file_path has been assigned
			// NOTE: Encountering an exception appears to cause the next call to fail
			if (tagem_module::file_path != nullptr)
				Py_DECREF(tagem_module::file_path);
			init_ytdl();
			return true;
		}
		
		failed = (unlikely(PyErr_Occurred() != nullptr));
		PyObj _file_path(tagem_module::file_path);
		PyObj _json(tagem_module::json_metadata);
		if (not failed and (likely(_file_path.obj != nullptr))){
			// Might be nullptr if the url is valid but the video(s) has been removed
			_file_path.copy_str(out_fmt_as_input__resulting_fp_as_output);
			this->process_remote_video_metadata(user_id, file_id, _json.as_str());
		}
		}
		return failed;
	}
	
	template<typename FileIDType>
	bool get_remote_video_metadata(const UserIDIntType user_id,  const FileIDType file_id,  const char* const url){
		using namespace python;
		bool failed;
		GILLock gillock();
		{ // Ensures the GILLock destructor is called after all PyObj destructors
		PyDict<5> opts(
			"max_downloads", PyLong_FromLong(1),
			"quiet", Py_False,
			"forcejson", Py_True,
			"simulate", Py_True,
			"skip_download", Py_True
		);
		Py_INCREF(Py_False);
		Py_INCREF(Py_True);
		Py_INCREF(Py_True);
		Py_INCREF(Py_True);
		PyObj ytdl_instantiation(ytdl_obj.call(opts.obj));
		
		// Override to_stdout, so that the file path is written to a buffer instead of stdout
		ytdl_instantiation.set_attr("to_stdout", tagem_module::to_stdout);
		
		try {
			ytdl_instantiation.call_fn_void("download", PyObj(Py_BuildValue("[s]", url)).obj);
		} catch(const std::runtime_error& e){
			// NOTE: Encountering a Python exception seems to set ytdl_obj to nullptr
			init_ytdl();
			return true;
		}
		
		failed = (unlikely(PyErr_Occurred() != nullptr));
		PyObj _json(tagem_module::json_metadata);
		if (not failed){
			failed = this->process_remote_video_metadata(user_id, file_id, _json.as_str());
		}
		}
		return failed;
	}
#endif
	const char* guess_mimetype(const char* const path) const {
		return magic_file(magique, path);
	}
	
	std::string_view guess_null_mimetypes(const char* s){
		GET_USER_ID
		GREYLIST_USERS_WITHOUT_PERMISSION("exec_safe_tasks")
		
		const char* tbl_suffixes[] = {"", "_backup"};
		for (auto i = 0;  i < 2;  ++i){
			const char* const tbl_suffix = tbl_suffixes[i];
			this->mysql_query<0>(
				"SELECT f.", (i==0)?"id":"file", ", CONCAT(d.full_path, f.name)"
				"FROM file", tbl_suffix, " f "
				"JOIN dir d ON d.id=f.dir "
				"WHERE f.mimetype=0 "
				"AND d.device IN (0", _f::zip2, connected_local_devices.size(), ',', connected_local_devices, ")"
			);
			uint64_t f_id;
			const char* path;
			while(this->mysql_assign_next_row<0>(&f_id, &path)){
				const char* const mimetype_guess = guess_mimetype(path);
				if (unlikely(mimetype_guess == nullptr))
					continue;
				this->mysql_exec(
					"UPDATE file", tbl_suffix, " f "
					"JOIN mimetype m "
					"SET f.mimetype=m.id "
					"WHERE f.", (i==0)?"id":"file", "=", f_id, " "
					"AND LEFT(\"", mimetype_guess, "\",LENGTH(m.name))=m.name" // WARNING: Not escaped
				);
			}
		}
		return compsky::server::_r::post_ok;
	}
};

int main(int argc,  const char* const* argv){
	const char* const* const argv_orig = argv;
	
	curl::init();
	
	magique = magic_open(MAGIC_CONTINUE|MAGIC_ERROR|MAGIC_MIME);
	magic_load(magique, nullptr);
	
	int port_n = 0;
	std::vector<const char*> external_db_env_vars;
	external_db_env_vars.reserve(1);
	external_db_env_vars.push_back("TAGEM_MYSQL_CFG");
	while (*(++argv)){
		const char* const arg = *argv;
		if (arg[1] != 0)
			goto help;
		switch(*arg){
			case 'p':
				port_n = a2n<int>(*(++argv));
				break;
			case 'c':
				CACHE_DIR = *(++argv);
				CACHE_DIR_STRLEN = strlen(CACHE_DIR);
				break;
			case 'd':
				FILES_GIVEN_REMOTE_DIR = *(++argv);
				break;
			case 'f':
				FFMPEG_LOCATION = *(++argv);
				break;
			case '^':
				// TODO: Use better arg parsing
				EXTRACT_YTDL_ERA_SCRIPT_PATH = *(++argv);
				break;
			case 'x':
				external_db_env_vars.push_back(*(++argv));
				break;
			case 'Y':
				YTDL_FORMAT = *(++argv);
				break;
			default:
				goto help;
		}
	}
	
	for (const char* const env_var : external_db_env_vars){
		if (unlikely(getenv(env_var) == nullptr)){
			static_log("ERROR: Environmental variable is not set: ", env_var);
			goto help;
		}
	}
	
	if (unlikely(port_n == 0)){
		static_log("ERROR: Port not set");
		
		help:
		static_log(
			#include "help.txt"
		);
		return 1;
	}
	
#ifdef PYTHON
	python::init_ytdl();
#endif
	
	if (mysql_library_init(0, NULL, NULL))
		throw compsky::mysql::except::SQLLibraryInit();
	
	db_infos.reserve(external_db_env_vars.size());
#define DB_NAME2ID_JSON_INIT \
		HEADER__RETURN_CODE__OK \
		HEADER__CONTENT_TYPE__JSON \
		CACHE_CONTROL_HEADER \
		"\n" \
		"{\""
	std::string db_name2id_json = DB_NAME2ID_JSON_INIT;
	MYSQL_RES* res;
	MYSQL_ROW row;
	MYSQL* tagem_mysql_obj;
	for (unsigned i = 0;  i < external_db_env_vars.size();  ++i){
		const char* const db_env_name = external_db_env_vars.at(i);
		
		DatabaseInfo& db_info = db_infos.emplace_back(db_env_name, (i!=0));
		
		if (i == 0){
			tagem_mysql_obj = db_infos.at(0).get();
			continue;
		}
		
		char buf[200];
		compsky::mysql::query(tagem_mysql_obj, res, buf, "SELECT id FROM external_db WHERE name=\"", db_info.name(), "\"");
		unsigned id = 0;
		while(compsky::mysql::assign_next_row(res, &row, &id));
		db_name2id_json += std::to_string(id) + std::string("\":\"") + db_info.name() + std::string("\",\"");
		if (id == 0){
			static_log("External database not recorded in external_db table: ", db_info.name());
			return 1;
		}
		db_indx2id[i] = id;
		
		db_info.test_is_accessible_from_master_connection(tagem_mysql_obj,  buf);
	}
	if (db_name2id_json.size() != std::char_traits<char>::length(DB_NAME2ID_JSON_INIT))
		// If there is at least one element in this dictionary
		db_name2id_json.pop_back();
	db_name2id_json.back() = '}';
	compsky::server::_r::external_db_json = db_name2id_json.c_str();
	// NOTE: This appears to be bugged in docker builds, only returning the headers and '}'.
	
	DatabaseInfo& tagem_db_info = db_infos.at(0);
	initialise_tagem_db(tagem_mysql_obj);
	tagem_db_info.free(tagem_mysql_obj);
	
#ifdef PYTHON
	if (FILES_GIVEN_REMOTE_DIR != nullptr)
		python::import_view_remote_dir(tagem_db_info);
#endif
	
	UserIDIntType user_id;
	uint64_t id;
	const char* name;
	
	tagem_db_info.query_buffer(
		res,
		"SELECT "
			"u.id,"
			"u.name,"
			"IFNULL(GROUP_CONCAT(f2.name),\"\")" // WARNING: file2 variables must not include commas
		"FROM user u "
		"LEFT JOIN user2shown_file2 u2v ON u2v.user=u.id "
		"LEFT JOIN file2 f2 ON f2.id=u2v.file2 "
		"GROUP BY u.id"
	);
	user_auth::users.reserve(compsky::mysql::n_results<size_t>(res));
	char* allowed_file2_vars;
	while(compsky::mysql::assign_next_row__no_free(res, &row, &user_id, &name, &allowed_file2_vars))
		const user_auth::User& user = user_auth::users.emplace_back(name, user_id, allowed_file2_vars);
	
	MYSQL_RES* res2;
	tagem_db_info.query_buffer(
		res2,
		"SELECT CONCAT(\" file2_\", id, \" ON file2_\", id, \".file=f.id \")"
		"FROM file2"
	);
	left_join_unique_name_for_each_file2_var.reserve(compsky::mysql::n_results<size_t>(res2));
	while(compsky::mysql::assign_next_row__no_free(res2, &row, &name))
		left_join_unique_name_for_each_file2_var.push_back(name);
	
	MYSQL_RES* res3;
	tagem_db_info.query_buffer(
		res3,
		"SELECT CONCAT(\",',',IFNULL(file2_\", id, \".x,0)\")"
		"FROM file2"
	);
	select_unique_name_for_each_file2_var.reserve(compsky::mysql::n_results<size_t>(res3));
	while(compsky::mysql::assign_next_row__no_free(res3, &row, &name))
		select_unique_name_for_each_file2_var.push_back(name);
	
	MYSQL_RES* res4;
	tagem_db_info.query_buffer(
		res4,
		"SELECT id, name "
		"FROM device "
		"WHERE name LIKE '/%'"
	);
	connected_local_devices.reserve(compsky::mysql::n_results<size_t>(res4));
	while(compsky::mysql::assign_next_row(res4, &row, &id, &name)){
		if (not os::file_exists(name))
			continue;
		connected_local_devices.push_back(id);
		connected_local_devices_str += std::to_string(id) + ",";
	}
	connected_local_devices_str.pop_back();
	
	tagem_db_info.query_buffer(
		res4,
		"SELECT id "
		"FROM tag "
		"WHERE name=\"!!PART OF FILE!!\" "
		"LIMIT 1"
	);
	compsky::mysql::assign_next_row(res4, &row, &TAG__PART_OF_FILE__ID);
	// NOTE: The above result is not freed.
	
	{
	MYSQL_RES* res5;
	tagem_db_info.query_buffer(
		res5,
		"SELECT "
			"TABLE_NAME "
			// "COLUMN_NAME " // NOTE: This should always be "file"
		"FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE "
		"WHERE REFERENCED_TABLE_SCHEMA='tagem' "
		  "AND REFERENCED_TABLE_NAME='file' "
		  "AND REFERENCED_COLUMN_NAME='id' "
		  "AND TABLE_NAME NOT IN ('file2post','file2tag','file2thumbnail','file_backup','user2whitelist_file')"
	);
	const char* tbl_name;
	while(compsky::mysql::assign_next_row__no_free(res5, &row, &tbl_name)){
		tables_referencing_file_id.push_back(tbl_name);
	}
	}
	
	{
	MYSQL_RES* res5;
	tagem_db_info.query_buffer(
		res5,
		"SELECT id "
		"FROM tag "
		"WHERE name=\"Uploader\""
	);
	while(compsky::mysql::assign_next_row__no_free(res5, &row, &UPLOADER_TAG_ID));
	}
	
	compsky::server::Server<N_THREADS, handler_req_buf_sz, TagemResponseHandler>(port_n).run();
	
	for (DatabaseInfo& db_info : db_infos){
		db_info.close();
	}
	
	mysql_library_end();
	
	curl::clean();

	return 0;
}
