#define DEBUG

#define USE_VECTOR
#include <compsky/asciify/asciify.hpp>

#include "FrameDecoder.h"
#include "CStringCodec.h"
#include "skip_to_body.hpp"
#include "qry.hpp"
#include "protocol.hpp"
#include "verify_str.hpp"
#include "convert_str.hpp"
#include "str_utils.hpp"
#include "db_info.hpp"
#include "user_auth.hpp"
#include "static_response.hpp"
#include "proc.hpp"
#include "curl_utils.hpp"

#include "get_cookies.hpp"
#include "read_request.hpp"

#ifdef n_cached
# include "cache.hpp"
#endif

#include <compsky/mysql/query.hpp>
#include <compsky/mysql/qryqry.hpp>
#include <compsky/utils/is_str_dblqt_escaped.hpp>

#include <folly/init/Init.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>

#include <mutex>
#include <cstring> // for malloc

#include <filesystem> // for std::filesystem::copy_file

#ifndef NO_VIEW_DIR_FS
# include <dirent.h> // TODO: Replace with std::filesystem
# include <openssl/md5.h>
#endif

/*
 * The following initial contents of YTDL_FORMAT are copyright TheFrenchGhostys (https://gitlab.com/TheFrenchGhosty)
 * Modified content from https://gitlab.com/TheFrenchGhosty/TheFrenchGhostys-YouTube-DL-Archivist-Scripts
 * GPL v3: https://github.com/TheFrenchGhosty/TheFrenchGhostys-YouTube-DL-Archivist-Scripts/raw/291b526c82f10b980c09ee5da9b432a039a1f0b0/LICENSE
 */
const char* YTDL_FORMAT = "(bestvideo[vcodec^=av01][height=720][fps>30]/bestvideo[vcodec^=vp9.2][height=720][fps>30]/bestvideo[vcodec^=vp9][height=720][fps>30]/bestvideo[vcodec^=avc1][height=720][fps>30]/bestvideo[height=720][fps>30]/bestvideo[vcodec^=av01][height=720]/bestvideo[vcodec^=vp9.2][height=720]/bestvideo[vcodec^=vp9][height=720]/bestvideo[vcodec^=avc1][height=720]/bestvideo[height=720]/bestvideo[vcodec^=av01][height<720][height>=480][fps>30]/bestvideo[vcodec^=vp9.2][height<720][height>=480][fps>30]/bestvideo[vcodec^=vp9][height<720][height>=480][fps>30]/bestvideo[vcodec^=avc1][height<720][height>=480][fps>30]/bestvideo[height<720][height>=480][fps>30]/bestvideo[vcodec^=av01][height<720][height>=480]/bestvideo[vcodec^=vp9.2][height<720][height>=480]/bestvideo[vcodec^=vp9][height<720][height>=480]/bestvideo[vcodec^=avc1][height<720][height>=480]/bestvideo[height<720][height>=480]/bestvideo[vcodec^=av01][height<720][height>=360][fps>30]/bestvideo[vcodec^=vp9.2][height<720][height>=360][fps>30]/bestvideo[vcodec^=vp9][height<720][height>=360][fps>30]/bestvideo[vcodec^=avc1][height<720][height>=360][fps>30]/bestvideo[height<720][height>=360][fps>30]/bestvideo[vcodec^=av01][height<720][height>=360]/bestvideo[vcodec^=vp9.2][height<720][height>=360]/bestvideo[vcodec^=vp9][height<720][height>=360]/bestvideo[vcodec^=avc1][height<720][height>=360]/bestvideo[height<720][height>=360]/bestvideo[vcodec^=av01][height<720][height>=240][fps>30]/bestvideo[vcodec^=vp9.2][height<720][height>=240][fps>30]/bestvideo[vcodec^=vp9][height<720][height>=240][fps>30]/bestvideo[vcodec^=avc1][height<720][height>=240][fps>30]/bestvideo[height<720][height>=240][fps>30]/bestvideo[vcodec^=av01][height<720][height>=240]/bestvideo[vcodec^=vp9.2][height<720][height>=240]/bestvideo[vcodec^=vp9][height<720][height>=240]/bestvideo[vcodec^=avc1][height<720][height>=240]/bestvideo[height<720][height>=240]/bestvideo[vcodec^=av01][height<720][height>=144][fps>30]/bestvideo[vcodec^=vp9.2][height<720][height>=144][fps>30]/bestvideo[vcodec^=vp9][height<720][height>=144][fps>30]/bestvideo[vcodec^=avc1][height<720][height>=144][fps>30]/bestvideo[height<720][height>=144][fps>30]/bestvideo[vcodec^=av01][height<720][height>=144]/bestvideo[vcodec^=vp9.2][height<720][height>=144]/bestvideo[vcodec^=vp9][height<720][height>=144]/bestvideo[vcodec^=avc1][height<720][height>=144]/bestvideo[height<720][height>=144]/bestvideo)+(bestaudio[acodec^=opus]/bestaudio)/best";

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
	DISTINCT_F2T_TAG_IDS

#define TRUE 1
#define FALSE 0

#define GET_PAGE_N(terminator) \
	printf("  s == %.10s...\n", s); \
	const unsigned page_n = a2n<unsigned>(&s); \
	printf("  s == %.10s...\n", s); \
	fflush(stdout); \
	if(*s != terminator) \
		return _r::not_found; \
	++s;

#define CHECK_FOR_EXPECT_100_CONTINUE_HEADER \
	printf("CHECK_FOR_EXPECT_100_CONTINUE_HEADER %s\n", s); \
	if(SKIP_TO_HEADER(8,"Expect: ")(s) != nullptr) \
		return _r::expect_100_continue;

#define GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(var_decl, var, var_length, str_name, terminating_char) \
	BOOST_PP_IF(var_decl, const char* const,) var  = get_comma_separated_ints(&str_name, terminating_char); \
	if (var == nullptr) \
		return _r::not_found; \
	BOOST_PP_IF(var_decl, const size_t,) var_length  = (uintptr_t)str_name - (uintptr_t)var;

#define GET_USER \
	user_auth::User* user = user_auth::get_user(get_cookie(s, "username=")); \
	if (user == nullptr) \
		return _r::not_found;
#define GET_USER_ID \
	const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username=")); \
	if (user_id == user_auth::SpecialUserID::invalid) \
		return _r::not_found;
#define BLACKLIST_GUEST \
	if (user_id == user_auth::SpecialUserID::guest) \
		return _r::not_found;
#define GREYLIST_GUEST \
	if (user_id == user_auth::SpecialUserID::guest) \
		return _r::requires_login;

#define GET_DB_INFO \
	const unsigned db_id = a2n<unsigned>(&s); \
	const unsigned db_indx = external_db_id2indx(db_id); \
	if (db_indx == 0) \
		return _r::not_found; \
	DatabaseInfo& db_info = db_infos.at(db_indx);


#define GET_FILE2_VAR_NAME(s) \
	const char* const file2_var_name = s; \
	while((*s != ' ') and (*s != 0)) \
		++s; \
	const size_t file2_var_name_len = (uintptr_t)s - (uintptr_t)file2_var_name; \
	/* No need to check for empty string - the later function does that*/ \
	GET_USER \
	if (unlikely(not matches__left_up_to_space__right_up_to_comma_or_null(file2_var_name, user->allowed_file2_vars_csv))) \
		return _r::not_found;


#define SELECT_TAGS_INFOS_FROM_STUFF(...) \
	"SELECT " \
		"t.id," \
		"t.name," \
		"GROUP_CONCAT(IFNULL(p.thumbnail," NULL_IMG_SRC ") ORDER BY (1/(1+t2pt.depth))*(p.thumbnail IS NOT NULL) DESC LIMIT 1)," \
		"GROUP_CONCAT(p.cover ORDER BY (1/(1+t2pt.depth))*(p.cover!=\"\") DESC LIMIT 1)," \
		"IFNULL(A.n,0) " \
	"FROM _tag t " \
	"JOIN tag2parent_tree t2pt ON t2pt.tag=t.id " \
	"JOIN _tag p ON p.id=t2pt.parent " \
	"LEFT JOIN(" \
		"SELECT tag, COUNT(*) AS n " \
		"FROM file2tag " \
		/* NOTE: MySQL doesn't seem to optimise this often - usually faster if duplicate the limitations within this subquery */ \
		"WHERE tag IN(" \
			__VA_ARGS__ \
		")" \
		"AND file NOT IN" USER_DISALLOWED_FILES(user_id) \
		"GROUP BY tag" \
	")A ON A.tag=t.id "
#define WHERE_TAGS_INFOS(...) \
	"WHERE t.id IN(" __VA_ARGS__ ")" \
	"AND (t2pt.depth=0 OR p.thumbnail IS NOT NULL OR p.cover != \"\")" \
	"AND t.id NOT IN" USER_DISALLOWED_TAGS(user_id) \
	/* "AND p.id NOT IN" USER_DISALLOWED_TAGS(user_id)  Unnecessary */
#define TAGS_INFOS(...) \
	SELECT_TAGS_INFOS_FROM_STUFF(__VA_ARGS__) \
	WHERE_TAGS_INFOS(__VA_ARGS__) \
	"GROUP BY t.id "
#define TAGS_INFOS__WTH_DUMMY_WHERE_THING(...) \
	/* See NOTE #dkgja */ \
	SELECT_TAGS_INFOS_FROM_STUFF(__VA_ARGS__) \
	"AND t.id>0 " \
	WHERE_TAGS_INFOS(__VA_ARGS__) \
	"GROUP BY t.id "


typedef wangle::Pipeline<folly::IOBufQueue&,  std::string_view> RTaggerPipeline;

namespace _f {
	using namespace compsky::asciify::flag;
	constexpr static const Escape esc;
	constexpr static const esc::DoubleQuote esc_dblqt;
	constexpr static const AlphaNumeric alphanum;
	constexpr static const StrLen strlen;
	constexpr static const JSONEscape json_esc;
	constexpr static const Repeat repeat;
	constexpr static const Zip3 zip3;
	constexpr static const NElements n_elements;
	constexpr static const Hex hex;
	constexpr static const grammatical_case::Lower lower_case;
	constexpr static const grammatical_case::Upper upper_case;
	constexpr static const esc::SpacesAndNonAscii esc_spaces_and_non_ascii;
	constexpr static const esc::URI_until_space::Unescape unescape_URI_until_space;
}

FILE* EXTERNAL_CMDS_TO_RUN = stderr;

static
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

static bool regenerate_mimetype_json = true;
static bool regenerate_dir_json = true;
static bool regenerate_device_json = true;
static bool regenerate_protocol_json = true;
static bool regenerate_tag2parent_json = true;

std::vector<std::string> banned_client_addrs;

constexpr
bool is_local_file_or_dir(const char* const url){
	return (url[0] == '/');
}


namespace _r {
	static const char* mimetype_json;
	static const char* tag2parent_json;
	static const char* external_db_json;
	static const char* protocols_json;
	static const char* devices_json;
	static const char* protocol_json;
	
	std::mutex mimetype_json_mutex;
	std::mutex tag2parent_json_mutex;
	std::mutex external_db_json_mutex;
	std::mutex protocols_json_mutex;
	std::mutex devices_json_mutex;
	std::mutex protocol_json_mutex;
	
	
	namespace flag {
		struct Dict{};
		struct Arr{};
	};
	
	
	void asciifiis(const flag::Dict& _flag,  char*& itr,  const uint64_t* n,  const char* const* str1,  const char* const* str2,  const char* const* str3){
		compsky::asciify::asciify(
			itr,
			'"', *n, '"', ':', '[',
				'"', _f::esc, '"', *str1, '"', ',',
				'"', _f::esc, '"', *str2, '"', ',',
				'"', _f::esc, '"', *str3, '"',
			']', ','
		);
	}
	size_t strlens(const flag::Dict& _flag,  const uint64_t*,  const char* const* str1,  const char* const* str2,  const char* const* str3){
		return std::char_traits<char>::length("\"1234567890123456789\":[\"\",\"\",\"\"],") + 2*strlen(*str1) + 2*strlen(*str2) + 2*strlen(*str3);
	}
	
	void asciifiis(const flag::Dict& _flag,  char*& itr,  const uint64_t* n,  const char* const* str1,  const char* const* str2){
		compsky::asciify::asciify(
			itr,
			'"', *n, '"', ':', '[',
				'"', _f::esc, '"', *str1, '"', ',',
				'"', _f::esc, '"', *str2, '"',
			']', ','
		);
	}
	size_t strlens(const flag::Dict& _flag,  const uint64_t*,  const char* const* str1,  const char* const* str2){
		return std::char_traits<char>::length("\"1234567890123456789\":[\"\",\"\"],") + 2*strlen(*str1) + 2*strlen(*str2);
	}
	
	void asciifiis(const flag::Arr& _flag,  char*& itr,  const uint64_t* id,  const char** name,  const char** description,  const char** content){
		compsky::asciify::asciify(
			itr,
			'[',
				*id, ',',
				'"', _f::esc, '"', *name, '"', ',',
				'"', _f::esc, '"', *description, '"', ',',
				'"', _f::json_esc, *content, '"',
			']', ','
		);
	}
	
	void asciifiis(const flag::Arr& _flag,  char*& itr,  const uint64_t* m,  const uint64_t* n){
		compsky::asciify::asciify(
			itr,
			'[', *m, ',', *n, ']', ','
		);
	}
	size_t strlens(const flag::Arr& _flag,  const uint64_t*,  const uint64_t*){
		return std::char_traits<char>::length("[1234567890123456789,1234567890123456789],");
	}
	
	void asciifiis(const flag::Dict& _flag,  char*& itr,  const uint64_t* n,  const char* const* str){
		compsky::asciify::asciify(
			itr,
			'"', *n, '"', ':',
				'"', _f::esc, '"', *str, '"',
			','
		);
	}
	size_t strlens(const flag::Dict& _flag,  const uint64_t*,  const char* const* str){
		return std::char_traits<char>::length("\"1234567890123456789\":\"\",") + 2*strlen(*str);
	}
	
	void asciifiis(const flag::Dict& _flag,  char*& itr,  const char* const* str1,  const char* const* str2){
		compsky::asciify::asciify(
			itr,
			'"', _f::esc, '"', *str1, '"', ':',
				'"', _f::esc, '"', *str2, '"',
			','
		);
	}
	size_t strlens(const flag::Dict& _flag,  const char* const* str1,  const char* const* str2){
		return std::char_traits<char>::length("\"\":\"\",") + 2*strlen(*str1) + 2*strlen(*str2);
	}
	
	void asciifiis(const flag::Dict& _flag,  char*& itr,  const uint64_t* n,  const char* const* str0,  const unsigned* m,  const char* const* str1,  const char* const* str2){
		compsky::asciify::asciify(
			itr,
			'"', *n, '"', ':', '[',
					'"', _f::esc, '"', *str0, '"', ',',
					*m, ',',
					'"', _f::esc, '"', *str1, '"', ',',
					'"', _f::esc, '"', *str2, '"',
				']',
			','
		);
	}
	size_t strlens(const flag::Dict& _flag,  const uint64_t*,  const char* const* str0,  const unsigned*,  const char* const* str1,  const char* const* str2){
		return std::char_traits<char>::length("\"1234567890123456789\":[\"\",123456789,\"\",\"\"],") + 2*strlen(*str0) + 2*strlen(*str1) + 2*strlen(*str2);
	}
	
	void asciifiis(const flag::Dict& _flag,  char*& itr,  const uint64_t* n,  const char* const* str0,  const uint64_t* m){
		compsky::asciify::asciify(
			itr,
			'"', *n, '"', ':', '[',
					'"', _f::esc, '"', *str0, '"', ',',
					*m,
				']',
			','
		);
	}
	size_t strlens(const flag::Dict& _flag,  const uint64_t*,  const char* const* str0,  const uint64_t*){
		return std::char_traits<char>::length("\"1234567890123456789\":[\"\",1234567890123456789],") + 2*strlen(*str0);
	}
	
	constexpr
	char opener(const flag::Dict& _flag){
		return '{';
	}
	constexpr
	char opener(const flag::Arr& _flag){
		return '[';
	}
	constexpr
	char closer(const flag::Dict& _flag){
		return '}';
	}
	constexpr
	char closer(const flag::Arr& _flag){
		return ']';
	}
}

class RTaggerHandler : public wangle::HandlerAdapter<const std::string_view,  const std::string_view> {
  private:
	constexpr static const size_t buf_sz = 100 * 1024 * 1024; // 100 MiB
	char* buf;
	char* itr;
	
	static std::mutex mysql_mutex;
	MYSQL_RES* res;
	MYSQL_RES* res2;
	MYSQL_ROW row;
	
	constexpr
	uintptr_t buf_indx(){
		return (uintptr_t)this->itr - (uintptr_t)this->buf;
	}
	
	constexpr
	void reset_buf_index(){
		this->itr = this->buf;
	}
	
	inline
	char last_char_in_buf(){
		return *(this->itr - 1);
	}
	
	constexpr
	void move_itr_to_trailing_null(){
		while(*this->itr != 0)
			++this->itr;
	}
	
	template<typename... Args>
	void asciify(Args... args){
		compsky::asciify::asciify(this->itr,  args...);
	};
	
	void mysql_query_buf_db_by_id(DatabaseInfo& db_info,  const char* const _buf,  const size_t _buf_sz){
		this->mysql_mutex.lock();
		compsky::mysql::query_buffer(db_info.mysql_obj, this->res, _buf, _buf_sz);
		this->mysql_mutex.unlock();
	}
	
	void mysql_query_buf_db_by_id2(DatabaseInfo& db_info,  const char* const _buf,  const size_t _buf_sz){
		this->mysql_mutex.lock();
		compsky::mysql::query_buffer(db_info.mysql_obj, this->res2, _buf, _buf_sz);
		this->mysql_mutex.unlock();
	}
	
	void mysql_exec_buf_db_by_id(DatabaseInfo& db_info,  const char* const _buf,  const size_t _buf_sz){
		this->mysql_mutex.lock();
		compsky::mysql::exec_buffer(db_info.mysql_obj, _buf, _buf_sz);
		this->mysql_mutex.unlock();
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
	
	void mysql_query_buf(const char* const _buf,  const size_t _buf_sz){
		this->mysql_query_buf_db_by_id(db_infos.at(0), _buf, _buf_sz);
	}
	
	void mysql_query_buf(const char* const _buf){
		this->mysql_query_buf_db_by_id(db_infos.at(0), _buf, std::char_traits<char>::length(_buf));
	}
	
	void mysql_query_using_buf(){
		this->mysql_query_buf(this->buf, this->buf_indx());
	}
	
	void mysql_exec_using_buf_db_by_id(DatabaseInfo& db_info){
		this->mysql_mutex.lock();
		compsky::mysql::exec_buffer(db_info.mysql_obj, this->buf, this->buf_indx());
		this->mysql_mutex.unlock();
	}
	
	void mysql_exec_using_buf(){
		this->mysql_exec_using_buf_db_by_id(db_infos.at(0));
	}
	
	template<typename... Args>
	void mysql_query_db_by_id(DatabaseInfo& db_info,  Args... args){
		this->reset_buf_index();
		this->asciify(args...);
		this->mysql_query_buf_db_by_id(db_info, this->buf, this->buf_indx());
	}
	
	template<typename... Args>
	void mysql_query_db_by_id2(DatabaseInfo& db_info,  Args... args){
		this->reset_buf_index();
		this->asciify(args...);
		this->mysql_query_buf_db_by_id2(db_info, this->buf, this->buf_indx());
	}
	
	template<typename... Args>
	void mysql_exec_db_by_id(DatabaseInfo& db_info,  Args... args){
		this->reset_buf_index();
		this->asciify(args...);
		this->mysql_exec_buf_db_by_id(db_info, this->buf, this->buf_indx());
	}
	
	template<typename... Args>
	void mysql_query(Args... args){
		this->mysql_query_db_by_id(db_infos.at(0), args...);
	}
	
	template<typename... Args>
	void mysql_query2(Args... args){
		this->mysql_query_db_by_id2(db_infos.at(0), args...);
	}
	
	template<typename... Args>
	void mysql_query_after_itr(Args... args){
		char* const itr_init = this->itr;
		this->asciify(args...);
		this->mysql_query_buf_db_by_id(db_infos.at(0),  itr_init,  (uintptr_t)this->itr - (uintptr_t)itr_init);
		this->itr = itr_init;
	}
	
	template<typename... Args>
	void mysql_exec(Args... args){
		this->reset_buf_index();
		this->asciify(args...);
		this->mysql_exec_using_buf();
	}

	
	template<typename... Args>
	bool mysql_assign_next_row(Args... args){
		return compsky::mysql::assign_next_row(this->res, &this->row, args...);
	}
	
	template<typename... Args>
	bool mysql_assign_next_row2(Args... args){
		// WARNING: Shares row.
		return compsky::mysql::assign_next_row(this->res2, &this->row, args...);
	}
	
	template<typename... Args>
	bool mysql_assign_next_row__no_free(Args... args){
		return compsky::mysql::assign_next_row__no_free(this->res, &this->row, args...);
	}
	
	void mysql_free_res(){
		mysql_free_result(this->res);
	}
	
	void mysql_free_res2(){
		mysql_free_result(this->res2);
	}
	
	void mysql_seek(const int i){
		mysql_data_seek(this->res, i);
	}
	
	std::string_view get_buf_as_string_view(){
		return std::string_view(this->buf, this->buf_indx());
	}
	
#ifdef n_cached
	void add_buf_to_cache(const unsigned int which_cached_fn,  const uint64_t user_id,  const unsigned int n_requests = 1){
		cached_stuff::add(this->buf, this->buf_indx(), which_cached_fn, user_id, n_requests);
	}
#endif
	
	
	template<typename FlagType,  typename... Args>
	char* get_itr_from_buf(nullptr_t,  const FlagType& _flag,  const char* const _headers,  Args... args){
		size_t sz = 0;
		
		sz += std::char_traits<char>::length(_headers);
		sz += 1;
		while(this->mysql_assign_next_row__no_free(args...)){
			sz += _r::strlens(_flag, args...);;
		}
		sz += 1;
		sz += 1;
		
		void* buf = malloc(sz);
		if(unlikely(buf == nullptr))
			exit(4096);
		return reinterpret_cast<char*>(buf);
	}
	template<typename FlagType,  typename... Args>
	char* get_itr_from_buf(char** buf,  const FlagType,  const char* const,  Args...){
		return *buf;
	}
	void set_buf_to_itr(nullptr_t, char*){}
	void set_buf_to_itr(char** buf,  char* itr){
		*buf = itr;
	}
	void set_buf_to_itr(const char** buf,  char* itr){
		*buf = const_cast<const char*>(itr);
	}
	template<typename StackdBuf,  typename MallocdBuf,  typename FlagType,  typename... Args>
	void init_json(const StackdBuf stacked_itr,  const FlagType& _flag,  MallocdBuf mallocd_dst,  Args... args){
		/*
		 * stacked_itr is either nullptr or this->itr
		 * In the first case,  itr_init is a new malloc'd string that is assigned to mallocd_dst
		 * In the latter case, this->itr is incremented so that a string_view of this->buf can be replied
		 */
		
		using namespace _r;
		
		constexpr static const char* const _headers =
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			#include "headers/Cache-Control/1day.c"
			"\n"
		;
		
		char* const itr_init = get_itr_from_buf(stacked_itr, _flag, _headers, args...);
		char* itr = itr_init;
		
		compsky::asciify::asciify(itr, _headers);
		compsky::asciify::asciify(itr, opener(_flag));
		this->mysql_seek(0); // Reset to first result
		while(this->mysql_assign_next_row(args...)){
			asciifiis(_flag, itr, args...);
		}
		if (unlikely(*(itr - 1) == ','))
			// If there was at least one iteration of the loop...
			--itr; // ...wherein a trailing comma was left
		*(itr++) = closer(_flag);
		*itr = 0;
		
		set_buf_to_itr(mallocd_dst, itr_init);
		set_buf_to_itr(stacked_itr, itr);
	}
	
	void begin_json_response(){
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		);
	}
	
	struct QuoteAndEscape{};
	struct QuoteNoEscape{};
	struct NoQuote{};
	constexpr static QuoteAndEscape quote_and_escape{};
	constexpr static QuoteNoEscape quote_no_escape{};
	constexpr static NoQuote no_quote{};
	void asciify_json_list_response(const QuoteAndEscape,  const char** str){
		this->asciify(
			'"', _f::esc, '"',  *str, '"', ','
		);
	}
	void asciify_json_list_response(const QuoteNoEscape,  const char** str){
		this->asciify(
			'"', *str, '"', ','
		);
	}
	void asciify_json_list_response(const QuoteNoEscape,  const char** str1,  const QuoteNoEscape,  const char** str2){
		this->asciify(
			'[',
				'"',  *str1, '"', ',',
				'"',  *str2, '"',
			']', ','
		);
	}
	void asciify_json_list_response(const QuoteNoEscape,  const char** str1,  const QuoteAndEscape,  const char** str2){
		this->asciify(
			'[',
				'"',  *str1, '"', ',',
				'"',  _f::esc, '"', *str2, '"',
			']', ','
		);
	}
	void asciify_json_list_response(const QuoteAndEscape,  const char** str1,  const QuoteAndEscape,  const char** str2,  const QuoteAndEscape,  const char** str3){
		this->asciify(
			'[',
				'"',  _f::esc, '"', *str1, '"', ',',
				'"',  _f::esc, '"', *str2, '"', ',',
				'"',  _f::esc, '"', *str3, '"',
			']', ','
		);
	}
	void asciify_json_list_response(const NoQuote,  const char** str){
		this->asciify(
			*str, ','
		);
	}
	template<typename Flag>
	bool mysql_assign_next_row_for_json_list_response(const Flag flag,  const char** str){
		return this->mysql_assign_next_row(str);
	}
	template<typename Flag1,  typename Flag2>
	bool mysql_assign_next_row_for_json_list_response(const Flag1 flag1,  const char** str1,  const Flag2 flag2,  const char** str2){
		return this->mysql_assign_next_row(str1, str2);
	}
	template<typename Flag1,  typename Flag2,  typename Flag3>
	bool mysql_assign_next_row_for_json_list_response(const Flag1,  const char** str1,  const Flag2,  const char** str2,  const Flag3,  const char** str3){
		return this->mysql_assign_next_row(str1, str2, str3);
	}
	template<typename... Args>
	void write_json_list_response_into_buf(Args... args){
		this->begin_json_response();
		this->asciify('[');
		while(this->mysql_assign_next_row_for_json_list_response(args...)){
			this->asciify_json_list_response(args...);
		}
		if(this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify(']');
		*this->itr = 0;
	}
	
	bool user_can_access_dir(const UserIDIntType user_id, const uint64_t dir_id){
		this->mysql_query("SELECt id FROM _dir WHERE id=", dir_id, " AND id NOT IN" USER_DISALLOWED_DIRS(user_id));
		const bool rc = (this->mysql_assign_next_row());
		if (rc)
			this->mysql_free_res();
		return rc;
	}
	
	std::string_view parse_qry(const char* s){
		GET_USER_ID
		
		if (unlikely(skip_to_body(&s)))
			return _r::not_found;
		
		this->itr = this->buf;
		if (sql_factory::parse_into(this->itr, s, connected_local_devices_str, user_id) != sql_factory::successness::ok)
			return _r::post_not_necessarily_malicious_but_invalid;
		
		this->asciify(
			
		);
		
		this->mysql_query_buf(this->buf, strlen(this->buf)); // strlen used because this->itr is not set to the end
		
		const char* id;
		this->write_json_list_response_into_buf(this->no_quote, &id);
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view file_thumbnail(const char* md5hex){
		constexpr static const char* const prefix =
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/png.c"
			#include "headers/Cache-Control/1day.c"
			"Content-Length: "
		;
		constexpr static const size_t prefix_len = std::char_traits<char>::length(prefix);
		
		if (*md5hex == ' ')
			return _r::img_not_found;
		
		for (auto i = 0;  i < 32;  ++i){
			if (not is_valid_hex_char(md5hex[i]))
				return _r::not_found;
			this->buf[CACHE_DIR_STRLEN + i] = md5hex[i];
		}
		memcpy(this->buf,  CACHE_DIR,  CACHE_DIR_STRLEN);
		this->buf[CACHE_DIR_STRLEN + 32 + 0] = '.';
		this->buf[CACHE_DIR_STRLEN + 32 + 1] = 'p';
		this->buf[CACHE_DIR_STRLEN + 32 + 2] = 'n';
		this->buf[CACHE_DIR_STRLEN + 32 + 3] = 'g';
		this->buf[CACHE_DIR_STRLEN + 32 + 4] = 0;
		FILE* const f = fopen(this->buf, "rb");
		if (f == nullptr)
			return _r::img_not_found;
		
		struct stat st;
		stat(this->buf, &st);
		const size_t sz = st.st_size;
		
		memcpy(this->buf, prefix, prefix_len);
		char* itr = this->buf + prefix_len;
		compsky::asciify::asciify(itr,  sz, '\n', '\n');
		
		const size_t n_read = fread(itr, 1, sz, f);
		
		fclose(f);
		
		*(itr + sz) = 0;
		
		return std::string_view(this->buf,  sz + (uintptr_t)itr - (uintptr_t)this->buf);
	}
	
	std::string_view get_all_file_names_given_dir_id(const char* s){
		const uint64_t id = a2n<uint64_t>(s);
		
		this->mysql_query(
			"SELECT name "
			"FROM _file "
			"WHERE dir=", id
			// WARNING: No security filter
		);
		
		this->begin_json_response();
		this->asciify('[');
		const char* name;
		while(this->mysql_assign_next_row(&name))
			this->asciify('"', _f::esc, '"', name, '"', ',');
		if(this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify(']');
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view dir_info(const char* s){
		const uint64_t id = a2n<uint64_t>(s);
		
#ifdef n_cached
		if (const int indx = cached_stuff::from_cache(cached_stuff::dir_info, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
#endif
		
		GET_USER_ID
		
		this->begin_json_response();
		this->asciify('[');
		
		const char* id_str;
		const char* name;
		
		this->asciify('[');
		this->mysql_query_after_itr(
			"SELECT name "
			"FROM _dir "
			"WHERE id=", id, " "
			  "AND id NOT IN" USER_DISALLOWED_DIRS(user_id)
		);
		while(this->mysql_assign_next_row(&name))
			this->asciify_json_list_response(this->quote_and_escape, &name);
		if(this->last_char_in_buf() != ',')
			return _r::unauthorised;
		--this->itr;
		this->asciify(']');
		this->asciify(',');
		
		this->asciify('[');
		this->mysql_query_after_itr(
			"SELECT d.id, d.name "
			"FROM _dir d "
			"JOIN dir2parent_tree dt ON dt.parent=d.id "
			"WHERE dt.dir=", id, " "
			  "AND d.id NOT IN" USER_DISALLOWED_DIRS(user_id)
			"ORDER BY depth DESC"
		);
		while(this->mysql_assign_next_row(&id_str, &name))
			this->asciify_json_list_response(this->quote_no_escape, &id_str, this->quote_and_escape, &name);
		if(this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify(']');
		this->asciify(',');
		
		this->asciify('[');
		this->mysql_query_after_itr(
			"SELECT d.id, d.name "
			"FROM _dir d "
			"JOIN dir2parent_tree dt ON dt.dir=d.id "
			"WHERE dt.parent=", id, " "
			  "AND d.id NOT IN" USER_DISALLOWED_DIRS(user_id)
			"ORDER BY depth DESC"
		);
		while(this->mysql_assign_next_row(&id_str, &name))
			this->asciify_json_list_response(this->quote_no_escape, &id_str, this->quote_and_escape, &name);
		if(this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify(']');
		
		this->asciify(']');
		*this->itr = 0;
		
#ifdef n_cached
		this->add_buf_to_cache(cached_stuff::dir_info, id);
#endif
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view files_given_dir__filesystem(const char* s){
		GET_PAGE_N(' ')
#ifndef NO_VIEW_DIR_FS
		GET_USER_ID
		GREYLIST_GUEST
		
		if (unlikely(skip_to_body(&s)))
			return _r::not_found;
		
		const char* const dir_path = s;
		
		std::array<uint8_t, 16> hash;
		
		const bool is_local = is_local_file_or_dir(dir_path);
		
		DIR* dir;
		
		if (is_local){
			dir = opendir(dir_path);
			if (unlikely(dir == nullptr))
				return _r::server_error;
		}
		
		// Determine if files exist in the database - if so, supply all the usual data
		this->mysql_query(
			"SELECT f.name "
			"FROM _file f "
			"JOIN _dir d ON d.id=f.dir "
			"WHERE d.name=\"", _f::esc, '"', dir_path, "\" "
			"UNION "
			"SELECT f.name "
			"FROM file_backup f "
			"JOIN _dir d ON d.id=f.dir "
			"WHERE d.name=\"", _f::esc, '"', dir_path, "\" "
			// WARNING: No limits. Directories should aim to avoid having too many files in each (low thousands) to mitigate malicious requests
		);
		
		this->begin_json_response();
		this->asciify("[");
		
		if (not is_local){
			if (FILES_GIVEN_REMOTE_DIR != nullptr){
				char* const _buf = this->itr;
				char* _itr = _buf;
				compsky::asciify::asciify(_itr, page_n, '\0');
				const char* args[] = {FILES_GIVEN_REMOTE_DIR, _buf, dir_path, nullptr};
				if (proc::exec(60,  args,  STDOUT_FILENO,  this->itr,  RTaggerHandler::buf_sz - this->buf_indx())){
					return _r::server_error;
				}
				this->move_itr_to_trailing_null();
			}
		} else {
		
		this->asciify("\"0\",[");
		struct dirent* e;
		struct stat st;
		unsigned min = 100 * page_n;
		unsigned indx = 0;
		unsigned count = 100;
		while (((e=readdir(dir)) != 0) and (count != 0)){
			const char* const ename = e->d_name;
			
			if (ename == nullptr)
				continue;
			
			if (ename[0] == '.'){
				if (ename[1] == 0)
					continue;
				if (ename[1] == '.')
					continue;
			}
			
			if (e->d_type == DT_DIR){
				continue;
			}
			
			if (compsky::mysql::in_results<0>(ename, this->res))
				// If ename is equal to a string in the 2nd column of the results, it has already been recorded
				continue;
			
			if (++indx <= min)
				continue;
			
			--count;
			
			MD5_CTX md5_ctx;
			MD5_Init(&md5_ctx);
			compsky::asciify::asciify(this->file_path, "file://", _f::esc_spaces_and_non_ascii, dir_path, _f::esc_spaces_and_non_ascii, ename, '\0');
			MD5_Update(&md5_ctx, this->file_path, strlen(this->file_path));
			MD5_Final(hash.data(), &md5_ctx);
			
			compsky::asciify::asciify(this->file_path, dir_path, ename, '\0');
			
			stat(this->file_path, &st);
			this->asciify(
				// Should be equivalent to asciify_file_info
				'[',
					"\"/i/f/", _f::lower_case, _f::hex, hash, "\"", ',',
					0, ',',
					'"', _f::esc, '"', ename,   '"', ',',
					'"', '"', ',',
					'"', st.st_size, '"', ',',
					'"', st.st_ctime, '"', ',',
					'0', ',',
					'0', ',',
					'0', ',',
					'0', ',',
					'0', ',',
					'0', ',',
					'0', ',',
					'0', ',',
					'"', '"', ',',
					'"', '"',
				']',
				','
			);
		}
		closedir(dir);
		this->mysql_free_res();
		
		}
		
		if (this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify("],["); // Empty tags dictionary
		
		if (this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify("]]");
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
#else
		return _r::not_found;
#endif
	}
	
	std::string_view file_info(const char* s){
		const uint64_t id = a2n<uint64_t>(s);
		
#ifdef n_cached
		if (const int indx = cached_stuff::from_cache(cached_stuff::file_info, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
#endif
		
		GET_USER
		const UserIDIntType user_id = user->id;
		
		const size_t n = user->allowed_file2_vars.size();
		
		
		this->reset_buf_index();
		this->begin_json_response();
		this->asciify('[');
		
		
		this->asciify('[');
		this->mysql_query_after_itr(
			"SELECT "
				FILE_OVERVIEW_FIELDS("f.dir") ","
				"f.mimetype,"
				"CONCAT(\"0\"", _f::n_elements, n, select_unique_name_for_each_file2_var, ")"
			"FROM _file f "
			"LEFT JOIN file2tag f2t ON f2t.file=f.id "
			"LEFT JOIN file2post f2p ON f2p.file=f.id "
			JOIN_FILE_THUMBNAIL,
			_f::zip3, n, "LEFT JOIN file2", user->allowed_file2_vars, left_join_unique_name_for_each_file2_var,
			"WHERE f.id=", id, " "
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			  "AND f.dir NOT IN" USER_DISALLOWED_DIRS(user_id)
			"GROUP BY f.id"
		);
		const char* md5_hash;
		const char* dir_id;
		const char* file_name;
		const char* f_title;
		const char* file_sz;
		const char* file_added_timestamp;
		const char* file_origin_timestamp;
		const char* duration;
		const char* w;
		const char* h;
		const char* views;
		const char* likes;
		const char* dislikes;
		const char* fps;
		const char* external_db_and_post_ids;
		const char* tag_ids;
		const char* mimetype;
		const char* file2_values;
		while(this->mysql_assign_next_row(&md5_hash, &dir_id, &file_name, &f_title, &file_sz, &file_added_timestamp, &file_origin_timestamp, &duration, &w, &h, &views, &likes, &dislikes, &fps, &external_db_and_post_ids, &tag_ids, &mimetype, &file2_values)){
			this->asciify(
				'"', md5_hash, '"', ',',
				dir_id, ',',
				'"', _f::esc, '"', file_name, '"', ',',
				'"', _f::esc, '"', f_title,   '"', ',',
				'"', file_sz, '"', ',',
				file_added_timestamp, ',',
				file_origin_timestamp, ',',
				duration, ',',
				w, ',',
				h, ',',
				views, ',',
				likes, ',',
				dislikes, ',',
				fps, ',',
				'"', external_db_and_post_ids, '"', ',',
				'"', tag_ids, '"', ',',
				mimetype, ',',
				'"', file2_values, '"'
			);
		}
		if(this->last_char_in_buf() != '"')
			// No results - probably because the user hasn't the permission to view the file
			return _r::not_found;
		this->asciify(']');
		this->asciify(',');
		
		
		this->asciify('[');
		this->mysql_query_after_itr(
			"SELECT dir, name, mimetype "
			"FROM file_backup "
			"WHERE file=", id, " "
			  "AND dir NOT IN" USER_DISALLOWED_BACKUP_DIRS(user_id)
		);
		const char* backup_dir_id;
		const char* backup_file_name;
		const char* backup_mimetype;
		while(this->mysql_assign_next_row(&backup_dir_id, &backup_file_name, &backup_mimetype)){
			this->asciify(
				'[',
					'"', backup_dir_id, '"', ',',
					'"', _f::esc, '"', backup_file_name, '"', ',',
					'"', backup_mimetype, '"',
				']', ','
			);
		}
		if(this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify(']');
		this->asciify(',');
		
		
		this->asciify('{');
		this->mysql_query_after_itr(
			"SELECT d.id, d.name, d.device "
			"FROM _dir d "
			"WHERE d.id IN("
				"SELECT dir "
				"FROM _file "
				"WHERE id=", id, " "
				"UNION "
				"SELECT dir "
				"FROM file_backup "
				"WHERE file=", id,
			")"
			  "AND d.id NOT IN" USER_DISALLOWED_BACKUP_DIRS(user_id)
		);
		while(this->mysql_assign_next_row(&backup_dir_id, &backup_file_name, &backup_mimetype)){
			this->asciify(
				'"', backup_dir_id, '"', ':',
				'[',
					'"', _f::esc, '"', backup_file_name, '"', ',',
					'"', backup_mimetype, '"',
				']', ','
			);
		}
		if(this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify('}');
		
		
		this->asciify(']');
		*this->itr = 0;
		
#ifdef n_cached
		this->add_buf_to_cache(cached_stuff::file_info, id);
#endif
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view tags_given_file(const char* id_str){
		const uint64_t id = a2n<uint64_t>(id_str);
		
#ifdef n_cached
		if (const int indx = cached_stuff::from_cache(cached_stuff::tags_given_file, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
#endif
		
		this->mysql_query(
			"SELECT "
				"tag "
			"FROM file2tag "
			"WHERE file=", id, " "
			"LIMIT 1000"
		);
		
		const char* tag_id;
		this->write_json_list_response_into_buf(this->no_quote, &tag_id);
		
#ifdef n_cached
		this->add_buf_to_cache(cached_stuff::tags_given_file, id);
#endif
		
		return this->get_buf_as_string_view();
	}
	
	void asciify_tags_arr(){
		const char* id;
		const char* name;
		const char* thumb;
		const char* cover;
		const char* count;
		this->reset_buf_index();
		this->begin_json_response();
		this->asciify('[');
		while(this->mysql_assign_next_row(&id, &name, &thumb, &cover, &count)){
			this->asciify(
				'[',
					'"', id, '"', ',',
					'"', _f::esc, '"', name, '"', ',',
					'"', _f::esc, '"', thumb, '"', ',',
					'"', _f::esc, '"', cover, '"', ',',
					count,
				']', ','
			);
		}
		if(this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify(']');
	}
	
	void asciify_tags_dict(){
		const char* id;
		const char* name;
		const char* thumb;
		const char* cover;
		const char* count;
		this->asciify('{');
		while(this->mysql_assign_next_row2(&id, &name, &thumb, &cover, &count)){
			this->asciify(
				'"', id, '"', ':',
				'[',
					'"', _f::esc, '"', name, '"', ',',
					'"', _f::esc, '"', thumb, '"', ',',
					'"', _f::esc, '"', cover, '"', ',',
					count,
				']', ','
			);
		}
		if(this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify('}');
	}
	
	void asciify_file_info__no_end(){
		//const char* protocol_id;
		const char* md5_hex;
		//const char* dir_id;
		//const char* dir_name;
		const char* f_id;
		const char* f_name;
		const char* f_title;
		const char* f_sz;
		const char* file_added_timestamp;
		const char* file_origin_timestamp;
		const char* duration;
		const char* w;
		const char* h;
		const char* views;
		const char* likes;
		const char* dislikes;
		const char* fps;
		const char* external_db_and_post_ids;
		const char* tag_ids;
		this->begin_json_response();
		this->asciify("[\"0\",[");
		while(this->mysql_assign_next_row__no_free(&md5_hex, &f_id, &f_name, &f_title, &f_sz, &file_added_timestamp, &file_origin_timestamp, &duration, &w, &h, &views, &likes, &dislikes, &fps, &external_db_and_post_ids, &tag_ids)){
			this->asciify(
				'[',
					'"', md5_hex, '"', ',',
					//dir_id, ',',
					//'"', _f::esc, '"', dir_name, '"', ',',
					f_id, ',',
					'"', _f::esc, '"', f_name,   '"', ',',
					'"', _f::esc, '"', f_title,  '"', ',',
					'"', f_sz, '"', ',', // Integer as string because Javascript can't handle big integers
					file_added_timestamp, ',',
					file_origin_timestamp, ',',
					duration, ',',
					w, ',',
					h, ',',
					views, ',',
					likes, ',',
					dislikes, ',',
					fps, ',',
					'"', external_db_and_post_ids, '"', ',',
					'"', tag_ids, '"',
				']',
				','
			);
		}
	}
	
	void asciify_file_info(){
		this->asciify_file_info__no_end();
		this->mysql_free_res();
		if (this->last_char_in_buf() == ',')
			// If there was at least one iteration of the loop...
			--this->itr; // ...wherein a trailing comma was left
		this->asciify("],");
		this->asciify_tags_dict();
		this->asciify("]");
		*this->itr = 0;
	}
	
	std::string_view post__record_files(const char* s){
		uint64_t dir_id = a2n<uint64_t>(&s);
		if ((*s != ' ') or (dir_id == 0))
			return _r::not_found;
		
		CHECK_FOR_EXPECT_100_CONTINUE_HEADER
		
		GET_USER_ID
		GREYLIST_GUEST
		
		if (unlikely(skip_to_body(&s)))
			return _r::not_found;
		const char* const body = s;
		
		this->reset_buf_index();
		this->asciify(
			"INSERT INTO _file "
			"(user,dir,name)"
			"VALUES"
		);
		while(true){
			const char* const file_name_begin = s;
			if(unlikely(not compsky::utils::is_str_dblqt_escaped(s, ',', '\0')))
				return _r::not_found;
			const size_t file_name_len = (uintptr_t)s - (uintptr_t)file_name_begin;
			this->asciify('(', user_id, ',', dir_id, ',', _f::strlen, file_name_begin, file_name_len, ')', ',');
			if (*s == 0)
				break;
			++s; // Skip comma
		}
		if (unlikely(this->last_char_in_buf() != ','))
			// No file names were provided
			return _r::not_found;
		--this->itr; // Remove trailing comma
		this->asciify("ON DUPLICATE KEY UPDATE user=user");
		*this->itr = 0;
		
		this->mysql_exec_buf();
		
		// Now return a map of name to ID
		this->mysql_query("SELECT name, id FROM _file WHERE name IN(", body, ")");
		const char* id;
		const char* name;
		constexpr _r::flag::Dict dict;
		this->reset_buf_index();
		this->init_json(&this->itr, dict, nullptr, &name, &id);
		return this->get_buf_as_string_view();
	}
	
	std::string_view post__save_file(const char* s){
		uint64_t file_id = a2n<uint64_t>(&s);
		if(*s != '/')
			return _r::not_found;
		++s;
		const uint64_t dir_id  = a2n<uint64_t>(&s);
		if(*s != '/')
			return _r::not_found;
		++s;
		
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, ' ')
		GET_USER_ID
		
		if (unlikely(not this->user_can_access_dir(user_id, dir_id)))
			return _r::not_found;
		
		if (unlikely(skip_to_body(&s)))
			return _r::not_found;
		
		const char* const file_name = s;
		const char* const b4_file_contents = skip_to(s, '\n');
		const size_t file_name_length = (uintptr_t)b4_file_contents - (uintptr_t)file_name;
		
		if(unlikely(b4_file_contents == nullptr))
			return _r::not_found;
		
		const char* const file_contents = b4_file_contents + 1;
		
		if (file_id != 0)
			// Update an existing file
			return _r::not_implemented_yet;
		
		this->mysql_query(
			"SELECT CONCAT(d.name, \"", _f::esc, '"', _f::strlen, file_name_length, file_name, "\") "
			"FROM _dir d "
			"WHERE d.id=", dir_id
		);
		const char* path;
		if(unlikely(not this->mysql_assign_next_row(&path)))
			// Invalid dir_id
			return _r::not_found;
		
		FILE* f = fopen(path, "rb");
		if(unlikely(f != nullptr)){
			fclose(f);
			this->mysql_free_res();
			return 
				#include "headers/return_code/SERVER_ERROR.c"
				"\n"
				"File already exists"
			;
		}
		
		f = fopen(path, "wb");
		printf("Creating file: %s\n", path);
		this->mysql_free_res();
		if(unlikely(f == nullptr))
			return _r::server_error;
		fwrite(file_contents, 1, strlen(file_contents), f);
		fclose(f);
		
		this->mysql_query(
			"SELECT id "
			"FROM _file "
			"WHERE dir=", dir_id, " "
			  "AND name=\"", _f::esc, '"', _f::strlen, file_name_length, file_name, "\""
		);
		while(this->mysql_assign_next_row(&file_id));
		if (file_id != 0){
			fprintf(stderr, "Warning: File existed in DB but not on FS\n");
		} else {
			const unsigned mimetype_id = 17; // "text/plain"
			
			/*
			 * WARNING: Probably violates law of least surprise.
			 * If file exists on filesystem: nothing happens
			 * Else if file exists on DB:    file is created on FS and DB, and tagged
			 * Else if file not exist:       file is created on DB, and tagged
			 */
			this->mysql_exec(
				"INSERT INTO _file "
				"(dir,user,mimetype,name)"
				"VALUES(",
					dir_id, ',',
					user_id, ',',
					mimetype_id, ',',
					'"', _f::esc, '"', _f::strlen, file_name_length, file_name, '"',
				")"
			);
			
			this->mysql_query(
				"SELECT id "
				"FROM _file "
				"WHERE dir=", dir_id, " "
				"AND name=\"", _f::esc, '"', _f::strlen, file_name_length, file_name, "\""
			);
			while(this->mysql_assign_next_row(&file_id));
		}
		
		this->add_tags_to_files(user_id, tag_ids, tag_ids_len, file_id);
		
		return _r::post_ok;
	}
	
	std::string_view replace_file_path_and_set_old_path_as_backup(const char* s){
		const uint64_t file_id = a2n<uint64_t>(&s);
		++s;
		const uint64_t new_path__dir_id = a2n<uint64_t>(s);
		
		if ((file_id == 0) or (new_path__dir_id == 0))
			return _r::not_found;
		
		GET_USER_ID
		
		if (unlikely(skip_to_body(&s)))
			return _r::not_found;
		
		const char* const new_path__file_name = s;
		
		this->mysql_exec(
			"INSERT INTO file_backup"
			"(file,dir,name,mimetype,user)"
			"SELECT id, dir, name, mimetype,", user_id, " "
			"FROM _file "
			"WHERE id=", file_id, " "
			  "AND id NOT IN" USER_DISALLOWED_FILES(user_id)
		);
		// TODO: Catch duplicate key error. Should never happen.
		
		this->mysql_exec(
			"UPDATE _file "
			"SET dir=", new_path__dir_id, ","
				"name=\"", _f::esc, '"', new_path__file_name, "\""
			"WHERE id=", file_id, " "
			  "AND id NOT IN" USER_DISALLOWED_FILES(user_id)
		);
		
		return _r::post_ok;
	}
	
	std::string_view external_user_posts(const char* s,  const unsigned required_db_info_bool_indx,  const char* const tbl_name,  const char* const col_name){
		GET_DB_INFO
		++s;
		const uint64_t external_user_id = a2n<uint64_t>(s);
		
		if (not db_info.is_true(required_db_info_bool_indx))
			return _r::not_found;
		
		GET_USER_ID
		
		this->mysql_query_db_by_id(
			db_info,
			"SELECT GROUP_CONCAT(", col_name, ")"
			"FROM ", tbl_name, " "
			"WHERE user=", external_user_id
		);
		MYSQL_RES* const _post_ids_res = this->res;
		const char* post_ids;
		this->mysql_assign_next_row(&post_ids);
		if (post_ids == nullptr){
			// mysql_assign_next_row always returns a single row due to the GROUP_CONCAT function
			mysql_free_result(_post_ids_res);
			return _r::EMPTY_JSON_LIST;
		}
		
		this->reset_buf_index();
		this->mysql_query(
			"SELECT GROUP_CONCAT(f.id)"
			"FROM _file f "
			"JOIN file2post f2p ON f2p.file=f.id "
			"WHERE f2p.post IN (", post_ids, ")"
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			"LIMIT 100"
		);
		mysql_free_result(_post_ids_res);
		
		const char* file_ids;
		this->mysql_assign_next_row(&file_ids);
		if (file_ids == nullptr){
			// mysql_assign_next_row always returns a single row due to the GROUP_CONCAT function
			this->mysql_free_res();
			return _r::EMPTY_JSON_LIST;
		}
		
		this->begin_json_response();
		this->asciify(
			"["
				"\"", file_ids, "\""
			"]"
		);
		*this->itr = 0;
		
		this->mysql_free_res();
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view external_user_info(const char* s){
		// Viewing the user's liked posts and comments etc. is in a separate function
		
		GET_DB_INFO
		++s;
		const uint64_t user_id = a2n<uint64_t>(s);
		
		this->mysql_query_db_by_id(
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
		this->mysql_assign_next_row(&name, &full_name, &is_verified, &n_followers, &tag_ids);
		// mysql_assign_next_row always returns a single row due to the GROUP_CONCAT function
		if (name == nullptr){
			// No ushc user
			this->mysql_free_res();
			return _r::not_found;
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
		
		this->mysql_free_res();
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view external_cmnt_rm(const char* s){
		GET_DB_INFO
		++s;
		const uint64_t cmnt_id = a2n<uint64_t>(s);
		
		if (not db_info.is_true(DatabaseInfo::has_cmnt_tbl))
			return _r::not_found;
		
		this->mysql_exec_db_by_id(
			db_info,
			"UPDATE cmnt "
			"SET content=NULL "
			"WHERE id=", cmnt_id
		);
		
		return _r::post_ok;
	}
	
	std::string_view external_post_likes(const char* s){
		GET_DB_INFO
		++s;
		const uint64_t post_id = a2n<uint64_t>(s);
		
		if (not db_info.is_true(DatabaseInfo::has_post2like_tbl))
			return _r::EMPTY_JSON_LIST;
		
		this->mysql_query_db_by_id(
			db_info,
			"SELECT "
				"u.id,"
				"u.name "
			"FROM user u "
			"JOIN post2like p2l ON p2l.user=u.id "
			"WHERE p2l.post=", post_id
		);
		const char* username;
		const char* user_id;
		this->write_json_list_response_into_buf(this->quote_no_escape, &user_id, this->quote_no_escape, &username);
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view external_post_info(const char* s){
		// Data comes in two parts: data excluding comments, and then comments
		
		GET_DB_INFO
		++s;
		const uint64_t post_id = a2n<uint64_t>(s);
		
		char* const _buf_plus_offset = this->buf + 300;
		char* _itr_plus_offset = _buf_plus_offset;
		// Reserve the first part of this->buf for writing SQL queries, and use the rest for writing the response.
		
		compsky::asciify::asciify(
			_itr_plus_offset,
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
			"[["
		);
		
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
			
			const char* user = nullptr;
			const char* timestamp;
			const char* n_likes;
			const char* username;
			const char* text;
			while(this->mysql_assign_next_row(&user, &timestamp, &n_likes, &username, &text)){
				compsky::asciify::asciify(
					_itr_plus_offset,
						'"', user, '"', ',', // As a string because Javascript rounds large numbers (!!!)
						timestamp, ',',
						n_likes, ',',
						'"', username, '"', ',',
						'"', _f::json_esc, text, '"',
					"],["
				);
			}
			if (user == nullptr)
				return _r::not_found;
		}
		
		if (db_info.is_true(DatabaseInfo::has_cmnt_tbl)){
			this->mysql_query_db_by_id(
				db_info,
				"SELECT "
					"c.id,"
					"IFNULL(c.parent,0),"
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
				"ORDER BY c.parent ASC" // Put parentless comments first
			);
			
			const char* id;
			const char* parent;
			const char* user;
			const char* timestamp;
			const char* username;
			const char* content;
			while(this->mysql_assign_next_row(&id, &parent, &user, &timestamp, &username, &content)){
				compsky::asciify::asciify(
					_itr_plus_offset,
					'[',
						'"', id, '"', ',', // As a string because Javascript rounds large numbers (!!!)
						'"', parent, '"', ',', // As a string because Javascript rounds large numbers (!!!)
						'"', user, '"', ',',  // As a string because Javascript rounds large numbers (!!!)
						timestamp, ',',
						'"', username, '"', ',',
						'"', _f::json_esc, content, '"',
					']',
					','
				);
			}
			if (*(_itr_plus_offset - 1) == ',')
				// If there was at least one iteration of the loop...
				--_itr_plus_offset; // ...wherein a trailing comma was left
		}
		
		compsky::asciify::asciify(_itr_plus_offset, "]]");
		*_itr_plus_offset = 0;
		
		return std::string_view(_buf_plus_offset,  (uintptr_t)_itr_plus_offset - (uintptr_t)_buf_plus_offset);
	}
	
	std::string_view files_given_tag(const char* s){
		GET_PAGE_N('/')
		const uint64_t id = a2n<uint64_t>(s);
		
#ifdef n_cached
		if (const int indx = cached_stuff::from_cache(cached_stuff::files_given_tag, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
#endif
		
		GET_USER_ID
		
		this->mysql_query2(
			TAGS_INFOS__WTH_DUMMY_WHERE_THING("SELECT DISTINCT tag FROM file2tag WHERE file IN(SELECT DISTINCT file FROM file2tag WHERE tag=", id, ")")
			// NOTE: #dkgja No idea why this is necessary. But otherwise MySQL returns only a single row. Alternatively, a temporary table could be used, as it is only subqueries acting weird.
		);
		this->mysql_query_after_itr(
			"SELECT "
				FILE_OVERVIEW_FIELDS("f.id")
			"FROM _file f "
			"LEFT JOIN file2tag f2t ON f2t.file=f.id "
			JOIN_FILE_THUMBNAIL
			"LEFT JOIN file2post f2p ON f2p.file=f.id "
			"WHERE f.id IN ("
				"SELECT file "
				"FROM file2tag "
				"WHERE tag=", id,
			")"
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			"GROUP BY f.id "
			"LIMIT 100 "
			"OFFSET ", 100*page_n
		);
		
		this->asciify_file_info();
		
#ifdef n_cached
		this->add_buf_to_cache(cached_stuff::files_given_tag, id);
#endif
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view files_given_ids(const char* s){
		GET_PAGE_N('/')
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, file_ids, file_ids_len, s, ' ')
		GET_USER_ID
		
		this->mysql_query2(
			TAGS_INFOS("SELECT DISTINCT tag FROM file2tag WHERE file IN(", _f::strlen, file_ids, file_ids_len, ")")
		);
		this->mysql_query_after_itr(
			"SELECT "
				FILE_OVERVIEW_FIELDS("f.id")
			"FROM _file f "
			"LEFT JOIN file2tag f2t ON f2t.file=f.id "
			JOIN_FILE_THUMBNAIL
			"LEFT JOIN file2post f2p ON f2p.file=f.id "
			"WHERE f.id IN (", _f::strlen, file_ids, file_ids_len, ")"
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			"GROUP BY f.id "
			"ORDER BY FIELD(f.id,", _f::strlen, file_ids, file_ids_len, ")"
			"LIMIT 100 "
			"OFFSET ", 100*page_n
		);
		
		this->asciify_file_info();
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view tags_given_ids(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, ' ')
		GET_USER_ID
		
		this->mysql_query(
			TAGS_INFOS("", _f::strlen, tag_ids, tag_ids_len, "")
			"ORDER BY FIELD(t.id,", _f::strlen, tag_ids, tag_ids_len, ")"
			// WARNING: No limit
		);
		this->asciify_tags_arr();
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view dirs_given_ids(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, dir_ids, dir_ids_len, s, ' ')
		GET_USER_ID
		
		this->mysql_query(
			"SELECT "
				"d.id,"
				"d.name,"
				"d.device,"
				"COUNT(*)"
			"FROM _dir d "
			"JOIN _file f ON f.dir=d.id "
			"WHERE d.id IN (", _f::strlen, dir_ids, dir_ids_len, ")"
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			  DIR_TBL_USER_PERMISSION_FILTER(user_id)
			"GROUP BY d.id "
			"ORDER BY FIELD(d.id,", _f::strlen, dir_ids, dir_ids_len, ")"
			// WARNING: No limit
		);
		const char* id;
		const char* name;
		const char* device;
		const char* count;
		this->reset_buf_index();
		this->begin_json_response();
		this->asciify('[');
		while(this->mysql_assign_next_row(&id, &name, &device, &count)){
			this->asciify(
				'[',
					'"', id, '"', ',',
					'"', _f::esc, '"', name, '"', ',',
					'"', device, '"', ',',
					count,
				']', ','
			);
		}
		if(this->last_char_in_buf() == ',')
			--this->itr;
		this->asciify(']');
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view files_given_dir(const char* s){
		GET_PAGE_N('/')
		const uint64_t id = a2n<uint64_t>(s);
		
#ifdef n_cached
		if (const int indx = cached_stuff::from_cache(cached_stuff::files_given_dir, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
#endif
		
		GET_USER_ID
		
		this->mysql_query2(
			TAGS_INFOS("SELECT DISTINCT tag FROM file2tag WHERE file IN(SELECT DISTINCT id FROM _file WHERE dir=", id, ")")
		);
		this->mysql_query_after_itr(
			"SELECT "
				FILE_OVERVIEW_FIELDS("f.id")
			"FROM _file f "
			"LEFT JOIN file2tag f2t ON f2t.file=f.id "
			JOIN_FILE_THUMBNAIL
			"LEFT JOIN file2post f2p ON f2p.file=f.id "
			"WHERE f.dir=", id, " "
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			"GROUP BY f.id "
			"LIMIT 100 "
			"OFFSET ", 100*page_n
		);
		
		this->asciify_file_info();
		
#ifdef n_cached
		this->add_buf_to_cache(cached_stuff::files_given_dir, id);
#endif
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view files_given_value(const char* s){
		GET_PAGE_N('/')
		GET_FILE2_VAR_NAME(s)
		const auto user_id = user->id;
		
		this->mysql_query2(
			TAGS_INFOS("SELECT DISTINCT tag FROM file2tag WHERE file IN(SELECT DISTINCT file FROM file2", _f::strlen, file2_var_name, file2_var_name_len, ")")
		);
		this->mysql_query_after_itr(
			"SELECT "
				FILE_OVERVIEW_FIELDS("f.id")
			"FROM _file f "
			"JOIN file2", _f::strlen, file2_var_name, file2_var_name_len, " f2v ON f2v.file=f.id "
			"LEFT JOIN file2tag f2t ON f2t.file=f.id "
			JOIN_FILE_THUMBNAIL
			"LEFT JOIN file2post f2p ON f2p.file=f.id "
			"WHERE TRUE "
			  FILE_TBL_USER_PERMISSION_FILTER(user->id)
			"GROUP BY f.id "
			"LIMIT 100 "
			"OFFSET ", 100*page_n
			// No need to impose a limit - this is very quick
		);
		
		this->asciify_file_info();
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view get_allowed_file2_vars_json(const char* s){
		GET_USER
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
			"\"NONE,", user->allowed_file2_vars_csv, "\""
		);
		return this->get_buf_as_string_view();
	}
	
	std::string_view get_mimetype_json(const char* s){
		this->mysql_query_buf("SELECT id, name FROM mimetype");
		
		std::unique_lock lock(_r::mimetype_json_mutex);
		if (unlikely(regenerate_mimetype_json)){
			// WARNING: Race condition since init_json uses global mysql objects
			// TODO: Eliminate race with mutex
			regenerate_mimetype_json = false;
			uint64_t id;
			const char* str1;
			constexpr _r::flag::Dict dict;
			this->init_json(nullptr, dict, &_r::mimetype_json, &id, &str1);
		}
		return _r::mimetype_json;
	}
	
	template<typename... Args>
	std::string_view select2(const char tbl_alias,  const UserIDIntType user_id,  Args... name_args){
		this->reset_buf_index();
		this->asciify(
			"SELECT id, name "
			"FROM ", (tbl_alias=='d')?"_dir":"_tag", " "
			"WHERE name ", name_args..., "\" "
			"AND id NOT IN"
		);
		switch(tbl_alias){
			case 'd':
				this->asciify(USER_DISALLOWED_DIRS(user_id));
				break;
			default: /*case 't'*/
				this->asciify(USER_DISALLOWED_TAGS(user_id));
				break;
		}
		this->asciify("LIMIT 50"); // TODO: Tell client if results have been truncated
		
		try{
			this->mysql_query_using_buf();
		}catch(const compsky::mysql::except::SQLExec& e){
			std::cerr << e.what() << std::endl;
			return _r::EMPTY_JSON_LIST;
		}
		uint64_t id;
		const char* name;
		constexpr _r::flag::Dict dict;
		this->reset_buf_index();
		this->init_json(&this->itr, dict, nullptr, &id, &name);
		return this->get_buf_as_string_view();
	}
	
	std::string_view select2_regex(const char tbl_alias,  const char* s){
		const char* const qry = s;
		
		GET_USER_ID
		
		return this->select2(tbl_alias, user_id, "REGEXP \"", _f::esc_dblqt, _f::unescape_URI_until_space, _f::upper_case, qry);
		// NOTE: AFAIK, in URLs, one should have spaces as %20 before the ? and + after.
		// However, select2 encodes spaces as %20, not +, even though they are passed as URL parameters.
	}
	
	std::string_view get_device_json(const char* s){
		GET_USER_ID
		if (user_id != user_auth::SpecialUserID::guest){
			uint64_t id;
			const char* name;
			unsigned protocol;
			const char* embed_pre;
			const char* embed_post;
			constexpr _r::flag::Dict dict;
			this->mysql_query(
				"SELECT id, name, protocol, embed_pre, embed_post "
				"FROM _device "
				"WHERE id NOT IN" USER_DISALLOWED_DEVICES(user_id)
			);
			this->itr = this->buf;
			this->init_json(&this->itr, dict, nullptr, &id, &name, &protocol, &embed_pre, &embed_post);
			return this->get_buf_as_string_view();
		}
		
		std::unique_lock lock(_r::devices_json_mutex);
		if (unlikely(regenerate_device_json)){
			regenerate_device_json = false;
			uint64_t id;
			const char* name;
			unsigned protocol;
			const char* embed_pre;
			const char* embed_post;
			constexpr _r::flag::Dict dict;
			this->mysql_query_buf(
				"SELECT id, name, protocol, embed_pre, embed_post "
				"FROM _device "
				"WHERE id NOT IN" USER_DISALLOWED_DEVICES__COMPILE_TIME(GUEST_ID_STR)
			);
			this->init_json(nullptr, dict, &_r::devices_json, &id, &name, &protocol, &embed_pre, &embed_post);
		}
		return _r::devices_json;
	}
	
	std::string_view get_exec_json(const char* s){
		GET_USER_ID
		GREYLIST_GUEST
		
		uint64_t id;
		const char* name;
		const char* description;
		const char* content;
		constexpr _r::flag::Arr arr;
		this->mysql_query(
			"SELECT t.id, t.name, t.description, t.content "
			"FROM task t "
			"JOIN user2authorised_task u2t ON u2t.task=t.id "
			"WHERE u2t.user=", user_id
		);
		this->itr = this->buf;
		this->init_json(&this->itr, arr, nullptr, &id, &name, &description, &content);
		return this->get_buf_as_string_view();
	}
	
	std::string_view exec_task(const char* s){
		const unsigned task_id = a2n<unsigned>(s);
		
		GET_USER_ID
		BLACKLIST_GUEST
		
		this->mysql_query(
			"SELECT t.content "
			"FROM task t "
			"JOIN user2authorised_task u2t ON u2t.task=t.id "
			"WHERE t.id=", task_id, " "
			  "AND u2t.user=", user_id
		);
		const char* content = nullptr;
		this->mysql_assign_next_row(&content);
		
		if(content == nullptr)
			// User tried to execute a task they were not authorised to see
			return _r::not_found;
		
		const char* content_end = content;
		while(true){
			while((*content_end != 0) and (*content_end != ';'))
				++content_end;
			this->mysql_exec_buf(content,  (uintptr_t)content_end - (uintptr_t)content);
			if(*content_end == 0)
				break;
			content = ++content_end;
		}
		
		this->mysql_free_res();
		
		return _r::post_ok;
	}
	
	std::string_view get_protocol_json(const char* s){
		this->mysql_query_buf("SELECT id, name, \"\" FROM protocol");
		
		std::unique_lock lock(_r::protocol_json_mutex);
		if (unlikely(regenerate_protocol_json)){
			regenerate_protocol_json = false;
			uint64_t id; // unsigned, really - just can't justify creating another function for template
			const char* name;
			const char* empty; // To deliver it as id:[name] rather than id:name
			constexpr _r::flag::Dict dict;
			this->init_json(nullptr, dict, &_r::protocol_json, &id, &name, &empty);
		}
		return _r::protocol_json;
	}
	
	std::string_view get_tag2parent_json(const char* s){
		GET_USER_ID
		if (user_id != user_auth::SpecialUserID::guest){
			uint64_t id;
			uint64_t id2;
			constexpr _r::flag::Arr arr;
			this->mysql_query(
				"SELECT tag, parent "
				"FROM tag2parent t2p "
				"WHERE t2p.tag NOT IN" USER_DISALLOWED_TAGS(user_id)
				  "AND t2p.parent NOT IN" USER_DISALLOWED_TAGS(user_id)
			);
			this->itr = this->buf;
			this->init_json(&this->itr, arr, nullptr, &id, &id2);
			return this->get_buf_as_string_view();
		}
		
		std::unique_lock lock(_r::tag2parent_json_mutex);
		if (unlikely(regenerate_tag2parent_json)){
			regenerate_tag2parent_json = false;
			uint64_t id;
			uint64_t id2;
			constexpr _r::flag::Arr arr;
			this->mysql_query_buf(
				"SELECT tag, parent "
				"FROM tag2parent t2p "
				"WHERE t2p.tag NOT IN" USER_DISALLOWED_TAGS__COMPILE_TIME(GUEST_ID_STR)
				  "AND t2p.parent NOT IN" USER_DISALLOWED_TAGS__COMPILE_TIME(GUEST_ID_STR)
			);
			this->init_json(nullptr, arr, &_r::tag2parent_json, &id, &id2);
		}
		return _r::tag2parent_json;
	}
	
	constexpr
	std::string_view return_img(const char* s){
		switch(*(s++)){
			case 'f':
				switch(*(s++)){
					case '/':
						// /i/f/
						return this->file_thumbnail(s);
				}
				break;
		}
		return _r::not_found;
	}
	
	std::string_view stream_file(const char* s){
		constexpr static const size_t block_sz = 1024 * 1024 * 10;
		constexpr static const size_t stream_block_sz = 1024 * 1024; // WARNING: Will randomly truncate responses, usually around several MiBs // TODO: Increase this buffer size.
		constexpr static const size_t room_for_headers = 1000;
		static_assert(buf_sz  >  block_sz + room_for_headers); // 1000 is to leave room for moving headers around
		
		const uint64_t id = a2n<uint64_t>(&s);
		if (unlikely(id == 0))
			return _r::not_found;
		
		uint64_t dir_id = 0;
		if(*s == '/'){
			++s; // Skip trailing slash
			dir_id = a2n<uint64_t>(s);
			if (unlikely(dir_id == 0))
				return _r::not_found;
		}
		
		size_t from;
		size_t to;
		const GetRangeHeaderResult rc = get_range(s, from, to);
		if (unlikely(rc == GetRangeHeaderResult::invalid)){
			return _r::not_found;
		}
		
		if (unlikely( (to != 0) and (to <= from) ))
			return _r::not_found;
		
		GET_USER_ID
		
		this->mysql_query(
			"SELECT m.name, CONCAT(d.name, f", (dir_id==0)?"":"2", ".name) "
			"FROM _file f ",
			(dir_id==0)?"":"JOIN file_backup f2 ON f2.file=f.id ",
			"JOIN _dir d ON d.id=f", (dir_id==0)?"":"2", ".dir "
			"JOIN mimetype m ON m.id=f.mimetype "
			"WHERE f.id=", id, " "
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			  BACKUP_DIR_TBL_USER_PERMISSION_FILTER(user_id)
			  ,(dir_id==0)?" OR ":" AND d.id=", dir_id
		);
		const char* mimetype = nullptr;
		const char* file_path;
		while(this->mysql_assign_next_row__no_free(&mimetype, &file_path));
		if (mimetype == nullptr){
			this->mysql_free_res();
			return _r::not_found;
		}
		
		FILE* const f = fopen(file_path, "rb");
		if (f == nullptr){
			fprintf(stderr, "Cannot open file: %s\n", file_path);
			this->mysql_free_res();
			return _r::invalid_file;
		}
		
		static struct stat st;
		if (unlikely(stat(file_path, &st))){
			fprintf(stderr, "Cannot stat file: %s\n", file_path);
			this->mysql_free_res();
			return _r::server_error;
		}
		const size_t f_sz = st.st_size;
		
		if (unlikely(fseek(f, from, SEEK_SET))){
			this->mysql_free_res();
			return _r::server_error;
		}
		
		const size_t bytes_to_read = (rc == GetRangeHeaderResult::none) ? block_sz : ((to) ? (to - from) : stream_block_sz);
		const size_t bytes_read = fread(this->buf + room_for_headers,  1,  bytes_to_read,  f);
		fclose(f);
		
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
		
		this->mysql_free_res();
		
		return std::string_view(this->buf + room_for_headers - headers_len,  headers_len + bytes_read);
	}
	
	char file_path[4096];
	
	FunctionSuccessness dl_or_cp_file(const char* user_headers,  const UserIDIntType user_id,  const uint64_t dir_id,  const char* const file_id,  const char* const file_name,  const char* const url,  const bool overwrite_existing,  char* mimetype,  const bool is_ytdl){
		FunctionSuccessness rc = FunctionSuccessness::ok;
		const char* dir_name = nullptr;
		
		if (in_str(file_name, '/') and (file_id==nullptr) and not is_ytdl){
			// TODO: Allow for this
			rc = FunctionSuccessness::server_error;
			goto dl_or_cp_file__return;
		}
		
		this->mysql_query("SELECT name FROM _dir WHERE id=", dir_id); //, " AND id NOT IN " USER_DISALLOWED_DIRS(user_id));
		if (not this->mysql_assign_next_row(&dir_name)){
			// No visible directory with the requested ID
			// MySQL results already freed
			rc = FunctionSuccessness::malicious_request;
			goto dl_or_cp_file__return;
		}
		
		if (not endswith(dir_name, '/')){
			// TODO: Allow for this
			rc = FunctionSuccessness::server_error;
			goto dl_or_cp_file__return;
		}
		
		if (not is_local_file_or_dir(dir_name)){
			rc = FunctionSuccessness::malicious_request;
			goto dl_or_cp_file__return;
		}
		
		// If YTDL, then this->file_path is the template of the path of the output file; else it is the path of the output file
		compsky::asciify::asciify(this->file_path, dir_name, (is_ytdl or file_id==nullptr)?file_name:file_id, '\0');
		
		printf("dl_file %s %lu %s\n", (overwrite_existing)?">":"+", dir_id, url);
		
		this->mysql_free_res();
		
		// WARNING: Appears to freeze if the directory is not accessible (e.g. an unmounted external drive)
		// TODO: Check device is mounted
		
		if (is_local_file_or_dir(url)){
			if (is_ytdl)
				rc = FunctionSuccessness::malicious_request;
			else
				rc = (std::filesystem::copy_file(url, this->file_path)) ? FunctionSuccessness::ok : FunctionSuccessness::server_error;
		} else {
			if (is_ytdl){
				compsky::asciify::asciify(
					this->file_path,
					dir_name,
					"%(extractor)s-%(id)s.%(ext)s",
					'\0'
				);
				const char* ytdl_args[] = {"youtube-dl", "-q", "-o", this->file_path, "-f", YTDL_FORMAT, url, nullptr};
				if (proc::exec(60, ytdl_args, STDERR_FILENO, this->file_path)){
					rc = FunctionSuccessness::server_error;
					goto dl_or_cp_file__return;
				}
				this->file_path[sizeof(this->file_path)-1] = 0;
				char* const file_extension = skip_to_after<char>(this->file_path, "Requested formats are incompatible for merge and will be merged into ");
				if (file_extension != nullptr){
					replace_first_instance_of(file_extension, '.', '\0');
					
					compsky::asciify::asciify(
						this->file_path,
						dir_name,
						"%(extractor)s-%(id)s.", // Omit the file extension, as youtube-dl does not get the correct extension in this case when simulating (why force simulating then?!)
						'\0'
					);
				}
				
				// Now run youtube-dl a second time, to paste the output filename into this->file_path, because it has no option to print the filename without only simulating
				const char* ytdl2_args[] = {"youtube-dl", "--get-filename", "-o", this->file_path, "-f", YTDL_FORMAT, url, nullptr};
				if (proc::exec(3600, ytdl2_args, STDOUT_FILENO, this->file_path)){
					rc = FunctionSuccessness::server_error;
					goto dl_or_cp_file__return;
				}
				
				if (file_extension == nullptr){
					replace_first_instance_of(this->file_path, '\n', '\0');
				} else {
					replace_first_instance_of(this->file_path, '\n', file_extension, '\0');
				}
				printf("YTDL to: %s\n", this->file_path);
				
				rc = FunctionSuccessness::ok;
			} else
				rc = dl_file__curl(user_headers, url, this->file_path, overwrite_existing, mimetype);
		}
		
		dl_or_cp_file__return:
		// For possible cleanups
		
		return rc;
	}
	
	FunctionSuccessness dl_file(const char* user_headers,  const UserIDIntType user_id,  const uint64_t dir_id,  const char* const file_id,  const char* const file_name,  const char* const url,  const bool overwrite_existing,  char* mimetype,  const bool force_remote,  const bool is_ytdl){
		if (is_local_file_or_dir(url) and force_remote)
			return FunctionSuccessness::malicious_request;
		
		return this->dl_or_cp_file(user_headers, user_id, dir_id, file_id, file_name, url, overwrite_existing, mimetype, is_ytdl);
	}
	
	bool add_to_db__unpack_tpl(const char* const id_and_url,  const size_t id_and_url_length,  uint64_t& parent_id,  const char*& url,  size_t& url_length){
		url = id_and_url;
		parent_id = a2n<uint64_t>(&url);
		if(unlikely(*url != '\t'))
			return true;
		++url;
		url_length = id_and_url_length - ( (uintptr_t)url - (uintptr_t)id_and_url );
		return false;
	}
	
	void add_t_to_db(const UserIDIntType user_id,  const char* const parent_ids,  const size_t parent_ids_len,  const char* const tag_name,  const size_t tag_name_len){
		this->mysql_exec(
			"INSERT INTO _tag "
			"(name,user)"
			"SELECT \"", _f::esc, '"', _f::strlen,  tag_name_len,  tag_name, "\",", user_id, " "
			"FROM _tag "
			"WHERE NOT EXISTS"
			"(SELECT id FROM _tag WHERE name=\"", _f::esc, '"', _f::strlen, tag_name_len,  tag_name, "\")"
			  "AND id NOT IN" USER_DISALLOWED_TAGS(user_id)
			"LIMIT 1"
		);
		this->mysql_exec(
			"INSERT INTO tag2parent "
			"(tag, parent, user)"
			"SELECT t.id, p.id,", user_id, " "
			"FROM _tag t "
			"JOIN _tag p "
			"WHERE p.id IN (", _f::strlen, parent_ids, parent_ids_len, ") "
			  "AND t.name=\"", _f::esc, '"', _f::strlen, tag_name_len,  tag_name, "\" "
			  "AND t.id NOT IN " USER_DISALLOWED_TAGS(user_id)
			  "AND p.id NOT IN " USER_DISALLOWED_TAGS(user_id)
			"ON DUPLICATE KEY UPDATE tag=tag"
		);
		this->mysql_exec(
			"INSERT INTO tag2parent_tree (tag, parent, depth) "
			"SELECT * "
			"FROM("
				"SELECT id AS id, id AS parent, 0 AS depth "
				"FROM _tag "
				"WHERE name=\"", _f::esc, '"', _f::strlen, tag_name_len,  tag_name, "\" "
				"UNION "
				"SELECT t.id, t2pt.parent, t2pt.depth+1 "
				"FROM _tag t "
				"JOIN tag2parent_tree t2pt "
				"WHERE t.name=\"", _f::esc, '"', _f::strlen, tag_name_len,  tag_name, "\" "
				"AND t2pt.parent IN (", _f::strlen, parent_ids, parent_ids_len,   ")"
				"AND t2pt.tag NOT IN" USER_DISALLOWED_TAGS(user_id)
			")A "
			"ON DUPLICATE KEY UPDATE depth=LEAST(tag2parent_tree.depth, A.depth)"
		);
	}
	
	bool add_D_to_db(const UserIDIntType user_id,  const char* const id_and_url,  const size_t id_and_url_len){
		const char* url;
		size_t url_len;
		uint64_t protocol;
		if (unlikely(this->add_to_db__unpack_tpl(id_and_url, id_and_url_len, protocol, url, url_len)))
			return true;
		
		this->mysql_exec(
			"INSERT INTO _device "
			"(protocol, name,user)"
			"SELECT ", protocol, ",\"", _f::esc, '"', _f::strlen,  url_len,  url, "\",", user_id, " "
			"FROM _device "
			"WHERE NOT EXISTS"
			"(SELECT id FROM _device WHERE name=\"", _f::esc, '"', _f::strlen,  url_len,  url, "\")"
			"LIMIT 1"
		);
		regenerate_device_json = true;
		return false;
	}
	
	bool add_d_to_db(const UserIDIntType user_id,  const char* const id_and_url,  const size_t id_and_url_len){
		const char* url;
		size_t url_len;
		uint64_t device;
		if (unlikely(this->add_to_db__unpack_tpl(id_and_url, id_and_url_len, device, url, url_len)))
			return true;
		
		this->mysql_exec(
			"INSERT INTO _dir "
			"(device, name, user)"
			"SELECT ", device, ",\"", _f::esc, '"', _f::strlen,  url_len,  url, "\",", user_id, " "
			"FROM _dir "
			"WHERE NOT EXISTS"
			"(SELECT id FROM _dir WHERE name=\"", _f::esc, '"', _f::strlen,  url_len,  url, "\")"
			  "AND ", device, " NOT IN" USER_DISALLOWED_DEVICES(user_id)
			"LIMIT 1"
		);
		regenerate_dir_json = true;
		return false;
	}
	
	bool add_f_to_db(const UserIDIntType user_id,  const char* const tag_ids,  const size_t tag_ids_len,  const char* id_and_url,  const size_t id_and_url_len){
		const char* url;
		size_t url_len;
		uint64_t dir_id;
		if (unlikely(this->add_to_db__unpack_tpl(id_and_url, id_and_url_len, dir_id, url, url_len)))
			return true;
		
		this->mysql_exec(
			"INSERT INTO _file "
			"(dir, name, user)"
			"SELECT d.id,SUBSTR(\"", _f::esc, '"', _f::strlen, url_len, url, "\",LENGTH(d.name)+1),", user_id, " "
			"FROM _file f "
			"JOIN _dir d ON d.id=", dir_id, " "
			"WHERE NOT EXISTS"
			"(SELECT f.id FROM _file f JOIN _dir d ON d.id=f.dir WHERE d.id=", dir_id, " AND f.name=SUBSTR(\"", _f::esc, '"', _f::strlen, url_len, url, "\",LENGTH(d.name)+1))"
			  DIR_TBL_USER_PERMISSION_FILTER(user_id)
			"LIMIT 1"
		);
		this->mysql_exec(
			"INSERT INTO file2tag "
			"(file, tag, user)"
			"SELECT f.id, t.id,", user_id, " "
			"FROM _file f "
			"JOIN _dir d ON d.id=f.dir "
			"JOIN _tag t "
			"WHERE t.id IN (", _f::strlen, tag_ids, tag_ids_len, ") "
			  "AND f.name=SUBSTR(\"", _f::esc, '"', _f::strlen, url_len, url, "\",LENGTH(d.name)+1) "
			  "AND f.dir=", dir_id, " "
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			  DIR_TBL_USER_PERMISSION_FILTER(user_id)
			"ON DUPLICATE KEY UPDATE file=file"
		);
		
		return false;
	}
	
	std::string_view add_to_tbl(const char tbl,  const char* s){
		const char* tag_ids;
		size_t tag_ids_len;
		
		if((tbl == 'f') or (tbl == 't')){
			GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(FALSE, tag_ids, tag_ids_len, s, '/')
			++s; // Skip trailing slash
		}
		
		GET_USER_ID
		
		s = skip_to_post_data(s);
		if (s == nullptr)
			return _r::not_found;
		
		do {
			++s; // Skip trailing newline
			const char* const url = s;
			while ((*s != 0) and (*s != '\n'))
				++s;
			const size_t url_len = (uintptr_t)s - (uintptr_t)url;
			if (url_len == 0)
				return _r::not_found;
			switch(tbl){
				case 'f':
					if (unlikely(this->add_f_to_db(user_id, tag_ids, tag_ids_len, url, url_len)))
						return _r::not_found;
					break;
				case 'd':
					if (unlikely(this->add_d_to_db(user_id, url, url_len)))
						return _r::not_found;
					break;
				case 'D':
					if (unlikely(this->add_D_to_db(user_id, url, url_len)))
						return _r::not_found;
					break;
				case 't':
					this->add_t_to_db(user_id, tag_ids, tag_ids_len, url, url_len);
					break;
			}
			if (*s == 0)
				return _r::post_ok;
		} while(true);
	}
	
	std::string_view update_tag_thumbnail(const char* s){
		const uint64_t tag_id = a2n<uint64_t>(&s);
		if (tag_id == 0)
			return _r::not_found;
		
		++s; // Skip slash
		
		const char* const url = s;
		const size_t url_length = count_until(url, ' ');
		
		if(*s == 0)
			return _r::not_found;
		
		GET_USER_ID
		BLACKLIST_GUEST
		
		this->mysql_exec(
			"UPDATE _tag "
			"SET thumbnail=\"", _f::esc, '"', _f::strlen, url_length, url, "\" "
			"WHERE id=", tag_id
		);
		
		return _r::post_ok;
	}
	
	std::string_view archive_reddit_post(const char* s){
		const char* const permalink = s;
		while((*s != 0) and (*s != ' '))
			++s;
		if(*s == 0)
			return _r::not_found;
		const size_t permalink_length = (uintptr_t)s - (uintptr_t)permalink;
		fprintf(EXTERNAL_CMDS_TO_RUN, "rscrape-submission %.*s\n",  (int)permalink_length,  permalink);
		fflush(EXTERNAL_CMDS_TO_RUN);
		return _r::post_ok;
	}
	
	std::string_view post__set_file_title(const char* s){
		const uint64_t f_id = a2n<uint64_t>(&s);
		GET_USER_ID
		GREYLIST_GUEST
		
		if (unlikely(skip_to_body(&s)))
			return _r::not_found;
		
		this->mysql_exec(
			"UPDATE _file "
			"SET title=\"", _f::esc, '"', s, "\" "
			"WHERE id=", f_id, " "
			  "AND id NOT IN" USER_DISALLOWED_FILES(user_id)
		);
		
		return _r::post_ok;
	}
	
	std::string_view post__merge_files(const char* s){
		const uint64_t orig_f_id = a2n<uint64_t>(&s);
		++s; // Skip slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, dupl_f_ids, dupl_f_ids_len, s, ' ')
		GET_USER_ID
		
		// TODO: Check user against user2whitelisted_action
		
		this->mysql_exec("DELETE FROM file2tag WHERE file IN (", _f::strlen, dupl_f_ids, dupl_f_ids_len, ") AND tag IN (SELECT tag FROM file2tag WHERE file=", orig_f_id, ")");
		this->mysql_exec("INSERT INTO file2tag (file,tag,user) SELECT ", orig_f_id, ", tag, user FROM file2tag f2t WHERE file IN (", _f::strlen, dupl_f_ids, dupl_f_ids_len, ") ON DUPLICATE KEY update file2tag.file=file2tag.file");
		this->mysql_exec("DELETE FROM file2tag WHERE file IN (", _f::strlen, dupl_f_ids, dupl_f_ids_len, ")");
		
		this->mysql_exec("DELETE FROM file2thumbnail WHERE file IN (", _f::strlen, dupl_f_ids, dupl_f_ids_len, ")");
		
		this->mysql_exec("DELETE FROM file_backup WHERE file IN (", _f::strlen, dupl_f_ids, dupl_f_ids_len, ") AND dir IN (SELECT dir FROM file_backup WHERE file=", orig_f_id, ")");
		this->mysql_exec("UPDATE file_backup SET file=", orig_f_id, " WHERE file IN (", _f::strlen, dupl_f_ids, dupl_f_ids_len, ")");
		
		this->mysql_exec("INSERT INTO file_backup (file,dir,name,mimetype,user) SELECT ", orig_f_id, ", f.dir, f.name, f.mimetype, ", user_id, " FROM _file f WHERE f.id IN (", _f::strlen, dupl_f_ids, dupl_f_ids_len, ") ON DUPLICATE KEY UPDATE file=file"); // WARNING: I think if there's a duplicate key, something has gone wrong previously.
		this->mysql_exec("DELETE FROM file2post WHERE post IN (SELECT post FROM file2post WHERE file=", orig_f_id, ") AND file IN(", _f::strlen, dupl_f_ids, dupl_f_ids_len, ")");
		this->mysql_exec("UPDATE file2post SET file=", orig_f_id, " WHERE file IN(", _f::strlen, dupl_f_ids, dupl_f_ids_len, ")");
		this->mysql_exec("DELETE FROM _file WHERE id IN (", _f::strlen, dupl_f_ids, dupl_f_ids_len, ")");
		
		return _r::post_ok;
	}
	
	std::string_view post__backup_file(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, file_ids, file_ids_len, s, '/')
		++s; // Skip slash
		const uint64_t dir_id = a2n<uint64_t>(&s);
		if (dir_id == 0)
			return _r::not_found;
		
		//++s; // Do not skip slash, as it is skipped by the following macro
		const bool is_ytdl = (IS_STR_EQL(s, 5, "ytdl/"));
		
		const char* const url = s; // An URL which (if supplied) is used instead of the original file URL
		const size_t url_length = count_until(url, ' ');
		
		const char* const user_headers = s;
		
		GET_USER_ID
		
		// TODO: Hide this option for guests in the UI, and BLACKLIST_GUESTS in this function
		
		this->mysql_query("SELECT f.id, d.name, f.name FROM _file f JOIN _dir d ON d.id=f.dir WHERE f.id IN(", _f::strlen, file_ids, file_ids_len, ")");
		char orig_file_path[4096];
		const char* file_id_str;
		const char* orig_dir_name;
		const char* file_name;
		while(this->mysql_assign_next_row(&file_id_str, &orig_dir_name, &file_name)){
		
		if ((url_length != 0) and not is_ytdl)
			compsky::asciify::asciify(orig_file_path, _f::strlen, url, url_length, '\0');
		else
			compsky::asciify::asciify(orig_file_path, orig_dir_name, file_name, '\0');
		
		char mimetype[100] = {0};
		MYSQL_RES* const prev_res = this->res;
		const auto rc = this->dl_or_cp_file(user_headers, user_id, dir_id, file_id_str, file_name, orig_file_path, false, mimetype, is_ytdl);
		this->res = prev_res;
		if (rc != FunctionSuccessness::ok)
			return (rc == FunctionSuccessness::malicious_request) ? _r::not_found : _r::server_error;
		
		const bool is_mimetype_set = (mimetype[0]);
		
		this->mysql_exec(
			"INSERT INTO file_backup "
			"(file,dir,name,mimetype,user)"
			"SELECT ", file_id_str, ',', dir_id, ",SUBSTR(\"", _f::esc, '"', this->file_path, "\",LENGTH(d.name)+1),mt.id,", user_id, " "
			"FROM _file f "
			"JOIN _dir d "
			"LEFT JOIN ",
				(is_mimetype_set)?"mimetype":"ext2mimetype",
				" mt ON mt.name=SUBSTRING_INDEX(\"",
				_f::esc, '"',
				(is_mimetype_set)?mimetype:this->file_path, // TODO: Escape mimetype properly
				(is_mimetype_set)?"\",';',1":"\",'.',-1", ") "
			"WHERE f.id=", file_id_str, " "
			  "AND d.id=", dir_id, " "
			"ON DUPLICATE KEY UPDATE file_backup.mimetype=VALUES(mimetype)"
		);
		// WARNING: The above will crash if there is no such extension in ext2mimetype
		// This is deliberate, to tell me to add it to the DB.
		
		}
		
		return _r::post_ok;
	}
	
	std::string_view post__dl(const char* s){
		static char url_buf[4096];
		
		const uint64_t dir_id = a2n<uint64_t>(&s);
		++s;
		
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, '/')
		++s; // Skip slash
		
		const char* const user_headers = s;
		
		GET_USER_ID
		
		const bool is_ytdl = false; // TODO: Allow ytdl
		
		const char* url = s;
		unsigned n_errors = 0;
		while(true){
			const char c = *s;
			++s;
			switch(c){
				case 0: // unlikely
					return _r::not_found;
				case ' ':
				case ',':
					const size_t url_len = (uintptr_t)s - (uintptr_t)url - 1;
					memcpy(url_buf, url, url_len);
					url_buf[url_len] = 0;
					
					const char* itr = url_buf;
					const char* file_name = nullptr;
					const char* ext = nullptr;
					while(*itr != 0){
						if (*itr == '/')
							file_name = itr;
						else if (*itr == '.')
							ext = itr;
						++itr;
					}
					++file_name; // Skip the slash
					const bool is_html_file  =  (ext == nullptr)  or  (ext < file_name);
					
					char mimetype[MAX_MIMETYPE_SZ + 1] = {0};
					switch(this->dl_file(user_headers, user_id, dir_id, nullptr, file_name, url_buf, is_html_file, mimetype, true, is_ytdl)){
						case FunctionSuccessness::server_error:
							++n_errors;
						case FunctionSuccessness::ok:
							break;
						case FunctionSuccessness::malicious_request:
							return _r::not_found;
					}
					
					const bool is_mimetype_set = (mimetype[0]);
					
					this->mysql_exec(
						"INSERT INTO _file"
						"(name, dir, user, mimetype)"
						"VALUES(",
							'"', _f::esc, '"', file_name, '"', ',',
							dir_id, ',',
							user_id, ',',
							(is_mimetype_set)?"(SELECT id FROM mimetype WHERE name=SUBSTRING_INDEX(\"":"",
							(is_mimetype_set)?mimetype:"", // TODO: Escape mimetype properly
							(is_mimetype_set)?"\",';',1))":"0",
						")"
						"ON DUPLICATE KEY UPDATE dir=dir"
					);
					this->mysql_exec(
						"INSERT INTO file2tag"
						"(file, tag, user)"
						"SELECT f.id, t.id,", user_id, " "
						"FROM _file f "
						"JOIN _tag t "
						"WHERE f.name=\"", _f::esc, '"', file_name, "\" "
						  "AND f.dir=", dir_id, " "
						  "AND t.id IN (", _f::strlen, tag_ids, tag_ids_len,") "
						  FILE_TBL_USER_PERMISSION_FILTER(user_id)
						  TAG_TBL_USER_PERMISSION_FILTER(user_id)
						"ON DUPLICATE KEY UPDATE file=file"
					);
					
					url = s;
					if (c == ' ')
						goto post__dl_break2;
					break;
			}
		}
		post__dl_break2:
		
		return (n_errors) ? _r::server_error : _r::post_ok;
	}
	
	void tag_antiparentisation(const UserIDIntType user_id,  const char* const child_ids,  const char* const tag_ids,  const size_t child_ids_len,  const size_t tag_ids_len){
		this->mysql_exec(
			"DELETE t2p "
			"FROM tag2parent t2p "
			"JOIN _tag t ON t.id=t2p.tag "
			"JOIN _tag p ON p.id=t2p.parent "
			"WHERE t.id IN (", _f::strlen, child_ids, child_ids_len, ")"
			  "AND p.id IN (", _f::strlen, tag_ids,   tag_ids_len,   ")"
			  "AND t.id NOT IN" USER_DISALLOWED_TAGS(user_id)
			  // "AND p.id NOT IN" USER_DISALLOWED_TAGS(user_id) // Unnecessary
		);
		
		// TODO: Descendant tags etc
		
		regenerate_tag2parent_json = true;
	}
	
	void tag_parentisation(const UserIDIntType user_id,  const char* const child_ids,  const char* const tag_ids,  const size_t child_ids_len,  const size_t tag_ids_len){
		this->mysql_exec(
			"INSERT INTO tag2parent (tag, parent, user) "
			"SELECT t.id, p.id,", user_id, " "
			"FROM _tag t "
			"JOIN _tag p "
			"WHERE t.id IN (", _f::strlen, child_ids, child_ids_len, ")"
			  "AND p.id IN (", _f::strlen, tag_ids,   tag_ids_len,   ")"
			  "AND t.id NOT IN" USER_DISALLOWED_TAGS(user_id)
			  "AND p.id NOT IN" USER_DISALLOWED_TAGS(user_id)
			"ON DUPLICATE KEY UPDATE tag=tag"
		);
		
		this->mysql_exec(
			"INSERT INTO tag2parent_tree (tag, parent, depth) "
			"SELECT t.id, t2pt.parent, t2pt.depth+1 "
			"FROM _tag t "
			"JOIN _tag p "
			"JOIN tag2parent_tree t2pt ON t2pt.tag=p.id "
			"WHERE t.id IN (", _f::strlen, child_ids, child_ids_len, ")"
			  "AND p.id IN (", _f::strlen, tag_ids,   tag_ids_len,   ")"
			  "AND t.id NOT IN" USER_DISALLOWED_TAGS(user_id)
			  "AND p.id NOT IN" USER_DISALLOWED_TAGS(user_id)
			"ON DUPLICATE KEY UPDATE depth=LEAST(tag2parent_tree.depth, t2pt.depth+1)"
		);
		
		// Update all descendant tags
		this->mysql_exec(
			"INSERT INTO tag2parent_tree (tag, parent, depth) "
			"SELECT t2pt.tag, t2pt2.parent, t2pt.depth+1 "
			"FROM tag2parent_tree t2pt "
			"JOIN tag2parent_tree t2pt2 ON t2pt2.tag=t2pt.parent "
			"WHERE t2pt.tag IN (", _f::strlen, tag_ids, tag_ids_len, ")"
			  "AND t2pt.tag NOT IN" USER_DISALLOWED_TAGS(user_id)
			"ON DUPLICATE KEY UPDATE depth=LEAST(tag2parent_tree.depth, t2pt.depth+1)"
		);
		
		regenerate_tag2parent_json = true;
	}
	
	std::string_view post__add_parents_to_tags(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, parent_ids, parent_ids_len, s, ' ')
		GET_USER_ID
		
		this->tag_parentisation(user_id, tag_ids, parent_ids, tag_ids_len, parent_ids_len);
		
		return _r::post_ok;
	}
	
	std::string_view post__rm_parents_from_tags(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, parent_ids, parent_ids_len, s, ' ')
		GET_USER_ID
		
		this->tag_antiparentisation(user_id, tag_ids, parent_ids, tag_ids_len, parent_ids_len);
		
		return _r::post_ok;
	}
	
	std::string_view post__add_children_to_tags(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, child_ids, child_ids_len, s, ' ')
		GET_USER_ID
		
		this->tag_parentisation(user_id, child_ids, tag_ids, child_ids_len, tag_ids_len);
		
		return _r::post_ok;
	}
	
	std::string_view post__rm_children_from_tags(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, child_ids, child_ids_len, s, ' ')
		GET_USER_ID
		
		this->tag_antiparentisation(user_id, child_ids, tag_ids, child_ids_len, tag_ids_len);
		
		return _r::post_ok;
	}
	
	template<typename... Args>
	void add_tags_to_files(const UserIDIntType user_id,  const char* const tag_ids,  const size_t tag_ids_len,  Args... file_ids_args){
		this->mysql_exec(
			"INSERT INTO file2tag (tag, file, user) "
			"SELECT t.id,f.id,", user_id, " "
			"FROM _tag t "
			"JOIN _file f "
			"WHERE t.id IN (", _f::strlen, tag_ids,  tag_ids_len,  ")"
			  "AND f.id IN (", file_ids_args..., ")"
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			  TAG_TBL_USER_PERMISSION_FILTER(user_id)
			"ON DUPLICATE KEY UPDATE file=file"
		);
	}
	
	template<typename... Args>
	void rm_tags_from_files(const UserIDIntType user_id,  const char* const tag_ids,  const size_t tag_ids_len,  Args... file_ids_args){
		this->mysql_exec(
			"DELETE f2t "
			"FROM file2tag f2t "
			"JOIN _tag t ON t.id=f2t.tag "
			"WHERE t.id IN (", _f::strlen, tag_ids,  tag_ids_len,  ")"
			  "AND f2t.file IN (", file_ids_args..., ")"
			  TAG_TBL_USER_PERMISSION_FILTER(user_id)
		);
	}
	
	std::string_view post__rm_tags_from_files(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, file_ids, file_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, ' ')
		GET_USER_ID
		
		BLACKLIST_GUEST
		
		this->mysql_query(
			"SELECT id "
			"FROM file "
			"WHERE id IN (", _f::strlen, file_ids, file_ids_len, ")"
			  "AND id IN" USER_DISALLOWED_FILES(user_id)
		);
		bool unauthorised = false;
		while(this->mysql_assign_next_row(&unauthorised));
		if(unauthorised)
			return _r::not_found;
		
		this->rm_tags_from_files(user_id, tag_ids, tag_ids_len, _f::strlen, file_ids, file_ids_len);
		
		return _r::post_ok;
	}
	
	std::string_view post__add_tag_to_file(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, file_ids, file_ids_len, s, '/')
		++s; // Skip trailing slash
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, tag_ids, tag_ids_len, s, ' ')
		GET_USER_ID
		
		BLACKLIST_GUEST
		
		this->add_tags_to_files(user_id, tag_ids, tag_ids_len, _f::strlen, file_ids, file_ids_len);
		
		return _r::post_ok;
	}
	
	std::string_view post__add_var_to_file(const char* s){
		GET_COMMA_SEPARATED_INTS_AND_ASSERT_NOT_NULL(TRUE, file_ids, file_ids_len, s, '/')
		++s; // Skip trailing slash
		const uint64_t value = a2n<uint64_t>(&s);
		++s; // Skip trailing slash
		GET_FILE2_VAR_NAME(s)
		
		const UserIDIntType user_id = user->id;
		GREYLIST_GUEST
		
		this->mysql_exec(
			"INSERT INTO file2", _f::strlen, file2_var_name, file2_var_name_len, " "
			"(file,x)"
			"SELECT f.id,", value, " "
			"FROM _file f "
			"WHERE f.id IN(", _f::strlen, file_ids, file_ids_len, ")"
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			"ON DUPLICATE KEY UPDATE x=x"
		);
		
		return _r::post_ok;
	}
	
	std::string_view post__edit_tag_cmnt(const char* s){
		const uint64_t tag_id = a2n<uint64_t>(&s);
		++s;
		
		printf("Edit tag cmnt: %lu: %s\n", tag_id, s);
		
		return _r::post_ok;
	}
	
	constexpr
	std::string_view determine_response(const char* str){
		--str;
#		include "auto__server-determine-response.hpp"
		return _r::not_found;
	}
  public:
	RTaggerHandler()
	{
		this->buf = (char*)malloc(this->buf_sz);
		if(unlikely(this->buf == nullptr))
			// TODO: Replace with compsky::asciify::alloc
			exit(4096);
	}
	
	~RTaggerHandler(){
	}
	
		void read(Context* ctx,  const std::string_view msg) override {
			this->reset_buf_index();
			for(size_t i = 0;  i < msg.size()  &&  msg.data()[i] != '\n'; ++i){
				this->asciify(msg.data()[i]);
			}
			*this->itr = 0;
			const std::string client_addr = ctx->getPipeline()->getTransportInfo()->remoteAddr->getAddressStr();
			std::cout << client_addr << '\t' << this->buf << std::endl;
			const std::string_view v = likely(std::find(banned_client_addrs.begin(), banned_client_addrs.end(), client_addr) == banned_client_addrs.end()) ? this->determine_response(msg.data()) : _r::banned_client;
			if (unlikely(v == _r::not_found))
				banned_client_addrs.push_back(client_addr);
			write(ctx, v);
			close(ctx);
		}
};
std::mutex RTaggerHandler::mysql_mutex;

class RTaggerPipelineFactory : public wangle::PipelineFactory<RTaggerPipeline> {
	public:
		RTaggerPipeline::Ptr newPipeline(std::shared_ptr<folly::AsyncTransportWrapper> sock) override {
			auto pipeline = RTaggerPipeline::create();
			pipeline->addBack(wangle::AsyncSocketHandler(sock));
			pipeline->addBack(wangle::FrameDecoder());
			pipeline->addBack(wangle::CStringCodec());
			pipeline->addBack(RTaggerHandler());
			pipeline->finalize();
			return pipeline;
		}
};

int main(int argc,  char** argv){
	curl_global_init(CURL_GLOBAL_ALL);
	
	assert(matches__left_up_to_space__right_up_to_comma_or_null("foo bar","what,does,foo,they"));
	assert(matches__left_up_to_space__right_up_to_comma_or_null("foo bar","what,does,they,foo"));
	assert(matches__left_up_to_space__right_up_to_comma_or_null("foo bar","foo,what,does"));
	assert(not matches__left_up_to_space__right_up_to_comma_or_null("bar foo","who,does,foo"));
	
	char** dummy_argv = argv;
	int port_n = 0;
	std::vector<char*> external_db_env_vars;
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
			case 'x':
				external_db_env_vars.push_back(*(++argv));
				break;
			case 'X':
				EXTERNAL_CMDS_TO_RUN = fopen(*(++argv), "wb");
				if(unlikely(EXTERNAL_CMDS_TO_RUN == nullptr)){
					fprintf(stderr, "Cannot open file to write external commands to: %s\n", *argv);
					abort();
				}
				break;
			case 'Y':
				YTDL_FORMAT = *(++argv);
				break;
			default:
				goto help;
		}
	}
	
	if (port_n == 0){
		help:
		fprintf(
			stderr,
			#include "help.txt"
		);
		return 1;
	}
	
	int dummy_argc = 0;
	folly::Init init(&dummy_argc, &dummy_argv);
	
	if (mysql_library_init(0, NULL, NULL))
		throw compsky::mysql::except::SQLLibraryInit();
	
	db_infos.reserve(external_db_env_vars.size());
	std::string db_name2id_json =
		#include "headers/return_code/OK.c"
		#include "headers/Content-Type/json.c"
		#include "headers/Cache-Control/1day.c"
		"\n"
		"{\""
	;
	MYSQL_RES* res;
	MYSQL_ROW row;
	for (unsigned i = 0;  i < external_db_env_vars.size();  ++i){
		char* const db_env_name = external_db_env_vars.at(i);
		
		DatabaseInfo& db_info = db_infos.emplace_back(db_env_name, (i!=0));
		
		if (i == 0)
			continue;
		
		char buf[200];
		compsky::mysql::query(db_infos.at(0).mysql_obj, res, buf, "SELECT id FROM external_db WHERE name=\"", db_info.name(), "\"");
		unsigned id = 0;
		while(compsky::mysql::assign_next_row(res, &row, &id));
		db_name2id_json += std::to_string(id) + std::string("\":\"") + db_info.name() + std::string("\",\"");
		if (id == 0){
			fprintf(stderr,  "External database not recorded in external_db table: %s\n", db_info.name());
			return 1;
		}
		db_indx2id[i] = id;
		
		db_info.test_is_accessible_from_master_connection(db_infos.at(0).connection(),  buf);
	}
	db_name2id_json.pop_back();
	db_name2id_json.back() = '}';
	_r::external_db_json = db_name2id_json.c_str();
	
	UserIDIntType user_id;
	uint64_t id;
	const char* name;
	
	compsky::mysql::query_buffer(
		db_infos.at(0).mysql_obj,
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
	compsky::mysql::query_buffer(
		db_infos.at(0).mysql_obj,
		res2,
		"SELECT CONCAT(\" file2_\", id, \" ON file2_\", id, \".file=f.id \")"
		"FROM file2"
	);
	left_join_unique_name_for_each_file2_var.reserve(compsky::mysql::n_results<size_t>(res2));
	while(compsky::mysql::assign_next_row__no_free(res2, &row, &name))
		left_join_unique_name_for_each_file2_var.push_back(name);
	
	MYSQL_RES* res3;
	compsky::mysql::query_buffer(
		db_infos.at(0).mysql_obj,
		res3,
		"SELECT CONCAT(\",',',IFNULL(file2_\", id, \".x,0)\")"
		"FROM file2"
	);
	select_unique_name_for_each_file2_var.reserve(compsky::mysql::n_results<size_t>(res3));
	while(compsky::mysql::assign_next_row__no_free(res3, &row, &name))
		select_unique_name_for_each_file2_var.push_back(name);
	
	MYSQL_RES* res4;
	compsky::mysql::query_buffer(
		db_infos.at(0).mysql_obj,
		res4,
		"SELECT id, name "
		"FROM _device "
		"WHERE name LIKE '/%'"
	);
	connected_local_devices.reserve(compsky::mysql::n_results<size_t>(res4));
	while(compsky::mysql::assign_next_row(res4, &row, &id, &name)){
		FILE* const f = fopen(name, "rb");
		if (f == nullptr)
			continue;
		connected_local_devices.push_back(id);
		connected_local_devices_str += std::to_string(id) + ",";
	}
	connected_local_devices_str.pop_back();
	
	wangle::ServerBootstrap<RTaggerPipeline> server;
	server.childPipeline(std::make_shared<RTaggerPipelineFactory>());
	server.bind(port_n);
	server.waitForStop();
	
	for (DatabaseInfo& db_info : db_infos){
		db_info.close();
	}
	
	mysql_library_end();
	
	curl_global_cleanup();

	return 0;
}
