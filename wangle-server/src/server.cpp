#include "FrameDecoder.h"
#include "CStringCodec.h"
#include "skip_to_body.hpp"
#include "qry.hpp"
#include "streq.hpp"
#include "protocol.hpp"
#include "verify_str.hpp"
#include "convert_str.hpp"
#include "str_utils.hpp"
#include "db_info.hpp"
#include "help.hpp"
#include "user_auth.hpp"
#include "static_response.hpp"

#include "get_cookies.hpp"
#include "read_request.hpp"
#include "read_request_url.hpp"

#ifdef n_cached
# include "cache.hpp"
#endif

#include <compsky/mysql/query.hpp>
#include <compsky/asciify/asciify.hpp>

#include <folly/init/Init.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>

#include <mutex>
#include <cstring> // for malloc

#define FILE_THUMBNAIL "IFNULL(IFNULL(f2tn.x, CONCAT('/i/f/', LOWER(HEX(f2h.x)))), \"\"),"
#define JOIN_FILE_THUMBNAIL "LEFT JOIN file2thumbnail f2tn ON f2tn.file=f.id "
#define DISTINCT_F2P_DB_AND_POST_IDS "IFNULL(GROUP_CONCAT(DISTINCT CONCAT(f2p.db,\":\",f2p.post),\"\"), \"\")"
#define DISTINCT_F2T_TAG_IDS "IFNULL(GROUP_CONCAT(DISTINCT f2t.tag_id),\"\")"


#include <curl/curl.h>


typedef wangle::Pipeline<folly::IOBufQueue&,  std::string_view> RTaggerPipeline;

namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
	constexpr static const compsky::asciify::flag::AlphaNumeric alphanum;
	constexpr static const compsky::asciify::flag::StrLen strlen;
	constexpr static const compsky::asciify::flag::JSONEscape json_esc;
	//constexpr static const compsky::asciify::flag::MaxBufferSize max_sz;
}

static
std::vector<DatabaseInfo> db_infos;

static
unsigned db_indx2id[128];

const char* CACHE_DIR = nullptr;
size_t CACHE_DIR_STRLEN;

static bool regenerate_mimetype_json = true;
static bool regenerate_dir_json = true;
static bool regenerate_device_json = true;
static bool regenerate_protocol_json = true;
static bool regenerate_tag_json = true;
static bool regenerate_tag2parent_json = true;

enum FunctionSuccessness {
	ok,
	malicious_request,
	server_error,
	unimplemented,
	COUNT
};

std::vector<std::string> banned_client_addrs;

FunctionSuccessness dl_file__curl(const char* const url,  const char* const dst_pth,  const bool overwrite_existing){
	if (not overwrite_existing){
		if (fopen(dst_pth, "rb") != nullptr)
			return FunctionSuccessness::ok;
	}
	
	FILE* const f = fopen(dst_pth, "w");
	if (f == nullptr){
		fprintf(stderr,  "Cannot open file for writing: %s\n",  dst_pth);
		return FunctionSuccessness::server_error;
	}
	
	CURL* const handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, f);
	curl_easy_perform(handle);
	
	fclose(f);
	
	curl_easy_cleanup(handle);
	
	return FunctionSuccessness::ok;
}

namespace _r {
	static const char* mimetype_json;
	static const char* tags_json;
	static const char* tag2parent_json;
	static const char* external_db_json;
	static const char* dirs_json;
	static const char* protocols_json;
	static const char* devices_json;
	static const char* protocol_json;
	
	std::mutex mimetype_json_mutex;
	std::mutex tags_json_mutex;
	std::mutex tag2parent_json_mutex;
	std::mutex external_db_json_mutex;
	std::mutex dirs_json_mutex;
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

class RTaggerHandler : public wangle::HandlerAdapter<const char*,  const std::string_view> {
  private:
	constexpr static const size_t buf_sz = 100 * 1024 * 1024; // 100 MiB
	char* buf;
	char* itr;
	
	static std::mutex mysql_mutex;
	MYSQL_RES* res;
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
	
	template<typename... Args>
	void asciify(Args... args){
		compsky::asciify::asciify(this->itr,  args...);
	};
	
	void mysql_query_buf_db_by_id(DatabaseInfo& db_info,  const char* const _buf,  const size_t _buf_sz){
		this->mysql_mutex.lock();
		compsky::mysql::query_buffer(db_info.mysql_obj, this->res, _buf, _buf_sz);
		this->mysql_mutex.unlock();
	}
	
	void mysql_exec_buf_db_by_id(DatabaseInfo& db_info,  const char* const _buf,  const size_t _buf_sz){
		this->mysql_mutex.lock();
		compsky::mysql::exec_buffer(db_info.mysql_obj, _buf, _buf_sz);
		this->mysql_mutex.unlock();
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
	bool mysql_assign_next_row__no_free(Args... args){
		return compsky::mysql::assign_next_row__no_free(this->res, &this->row, args...);
	}
	
	void mysql_free_res(){
		mysql_free_result(this->res);
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
	void asciify_json_list_response(const QuoteNoEscape,  const char** str1,  const QuoteNoEscape,  const char** str2){
		this->asciify(
			'[',
				'"',  *str1, '"', ',',
				'"',  *str2, '"',
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
	
	std::string_view parse_qry(const char* s){
		if (unlikely(skip_to_body(&s)))
			return _r::not_found;
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
		this->itr = this->buf;
		if (sql_factory::parse_into(this->itr, s, user_id) != sql_factory::successness::ok)
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
	
	std::string_view dir_info(const char* id_str){
		const uint64_t id = a2n<uint64_t>(id_str);
		
#ifdef n_cached
		if (const int indx = cached_stuff::from_cache(cached_stuff::dir_info, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
#endif
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(id_str, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
		this->mysql_query(
			"SELECT name "
			"FROM _dir "
			"WHERE id=", id, " "
			  "AND d.id NOT IN" USER_DISALLOWED_DIRS(user_id)
		);
		
		const char* name;
		this->write_json_list_response_into_buf(this->quote_and_escape, &name);
		
#ifdef n_cached
		this->add_buf_to_cache(cached_stuff::dir_info, id);
#endif
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view file_info(const char* id_str){
		const uint64_t id = a2n<uint64_t>(id_str);
		
#ifdef n_cached
		if (const int indx = cached_stuff::from_cache(cached_stuff::file_info, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
#endif
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(id_str, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
		this->mysql_query(
			"SELECT "
				FILE_THUMBNAIL
				"f.dir,"
				"f.name,"
				DISTINCT_F2P_DB_AND_POST_IDS ","
				DISTINCT_F2T_TAG_IDS ","
				"f.mimetype,"
				"IFNULL(GROUP_CONCAT(DISTINCT CONCAT(d2.id, ':', f2.mimetype)),\"\")"
			"FROM _file f "
			"LEFT JOIN file2tag f2t ON f2t.file_id=f.id "
			"LEFT JOIN file2post f2p ON f2p.file=f.id "
			JOIN_FILE_THUMBNAIL
			"LEFT JOIN file2qt5md5 f2h ON f2h.file=f.id "
			"LEFT JOIN file_backup f2 ON f2.file=f.id "
			"LEFT JOIN _dir d2 ON d2.id=f2.dir "
			"WHERE f.id=", id, " "
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			  "AND f.dir NOT IN" USER_DISALLOWED_DIRS(user_id)
			  "AND d2.id NOT IN" USER_DISALLOWED_DIRS(user_id)
			"GROUP BY f.id"
		);
		
		const char* md5_hash;
		const char* dir_id;
		const char* file_name;
		const char* external_db_and_post_ids;
		const char* tag_ids;
		const char* mimetype;
		const char* backup_dir_ids;
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		);
		this->asciify('[');
		while(this->mysql_assign_next_row(&md5_hash, &dir_id, &file_name, &external_db_and_post_ids, &tag_ids, &mimetype, &backup_dir_ids)){
			this->asciify(
				'"', md5_hash, '"', ',',
				dir_id, ',',
				'"', _f::esc, '"', file_name, '"', ',',
				'"', external_db_and_post_ids, '"', ',',
				'"', tag_ids, '"', ',',
				'"', mimetype, '"', ',',
				'"', backup_dir_ids, '"'
			);
		}
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
				"tag_id "
			"FROM file2tag "
			"WHERE file_id=", id, " "
			"LIMIT 1000"
		);
		
		const char* tag_id;
		this->write_json_list_response_into_buf(this->no_quote, &tag_id);
		
#ifdef n_cached
		this->add_buf_to_cache(cached_stuff::tags_given_file, id);
#endif
		
		return this->get_buf_as_string_view();
	}
	
	void asciify_file_info(){
		//const char* protocol_id;
		const char* md5_hex;
		//const char* dir_id;
		//const char* dir_name;
		const char* f_id;
		const char* f_name;
		const char* external_db_and_post_ids;
		const char* tag_ids;
		this->begin_json_response();
		this->asciify('[');
		while(this->mysql_assign_next_row(&md5_hex, &f_id, &f_name, &external_db_and_post_ids, &tag_ids)){
			this->asciify(
				'[',
					'"', md5_hex, '"', ',',
					//dir_id, ',',
					//'"', _f::esc, '"', dir_name, '"', ',',
					f_id, ',',
					'"', _f::esc, '"', f_name,   '"', ',',
					'"', external_db_and_post_ids, '"', ',',
					'"', tag_ids, '"',
				']',
				','
			);
		}
		if (this->last_char_in_buf() == ',')
			// If there was at least one iteration of the loop...
			--this->itr; // ...wherein a trailing comma was left
		this->asciify(']');
		*this->itr = 0;
	}
	
	std::string_view replace_file_path_and_set_old_path_as_backup(const char* s){
		const unsigned file_id = a2n<unsigned>(&s);
		++s;
		const uint64_t new_path__dir_id = a2n<uint64_t>(s);
		
		if ((file_id == 0) or (new_path__dir_id == 0))
			return _r::not_found;
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
		if (unlikely(skip_to_body(&s)))
			return _r::not_found;
		
		const char* const new_path__file_name = s;
		
		this->mysql_exec(
			"INSERT INTO file_backup"
			"(file,dir,name,mimetype,user)"
			"SELECT id, dir, name, mimetype,", user_id, " "
			"FROM _file "
			"WHERE id=", file_id, " "
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
		);
		// TODO: Catch duplicate key error. Should never happen.
		
		this->mysql_exec(
			"UPDATE _file "
			"SET dir=", new_path__dir_id, ","
				"name=\"", _f::esc, '"', new_path__file_name, "\""
			"WHERE id=", file_id, " "
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
		);
		
		return _r::post_ok;
	}
	
	std::string_view external_user_posts(const char* s,  const unsigned required_db_info_bool_indx,  const char* const tbl_name,  const char* const col_name){
		const unsigned db_indx = a2n<unsigned>(&s);
		++s;
		const uint64_t external_user_id = a2n<uint64_t>(s);
		
		if ((db_indx == 0) or (db_indx >= db_infos.size()))
			return _r::not_found;
		
		const unsigned db_id = db_indx2id[db_indx];
		
		DatabaseInfo& db_info = db_infos.at(db_id);
		
		if (not db_info.is_true(required_db_info_bool_indx))
			return _r::not_found;
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
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
		
		const unsigned db_indx = a2n<unsigned>(&s);
		++s;
		const uint64_t user_id = a2n<uint64_t>(s);
		
		if ((db_indx == 0) or (db_indx >= db_infos.size()))
			return _r::not_found;
		
		const unsigned db_id = db_indx2id[db_indx];
		
		DatabaseInfo& db_info = db_infos.at(db_id);
		
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
		const unsigned db_indx = a2n<unsigned>(&s);
		++s;
		const uint64_t cmnt_id = a2n<uint64_t>(s);
		
		if ((db_indx == 0) or (db_indx >= db_infos.size()))
			return _r::not_found;
		
		const unsigned db_id = db_indx2id[db_indx];
		DatabaseInfo& db_info = db_infos.at(db_id);
		
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
		const unsigned db_indx = a2n<unsigned>(&s);
		++s;
		const uint64_t post_id = a2n<uint64_t>(s);
		
		if ((db_indx == 0) or (db_indx >= db_infos.size()))
			return _r::not_found;
		
		const unsigned db_id = db_indx2id[db_indx];
		
		DatabaseInfo& db_info = db_infos.at(db_id);
		
		if (not db_info.is_true(DatabaseInfo::has_post2like_tbl))
			return _r::EMPTY_JSON_LIST;
		
		this->mysql_query_db_by_id(
			db_info,
			"SELECT "
				"u.name,"
				"u.id "
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
		
		const unsigned db_indx = a2n<unsigned>(&s);
		++s;
		const uint64_t post_id = a2n<uint64_t>(s);
		
		if ((db_indx == 0) or (db_indx >= db_infos.size()))
			return _r::not_found;
		
		const unsigned db_id = db_indx2id[db_indx];
		
		DatabaseInfo& db_info = db_infos.at(db_id);
		
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
	
	std::string_view files_given_tag(const char* id_str){
		const uint64_t id = a2n<uint64_t>(id_str);
		
#ifdef n_cached
		if (const int indx = cached_stuff::from_cache(cached_stuff::files_given_tag, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
#endif
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(id_str, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
		this->mysql_query(
			"SELECT "
				FILE_THUMBNAIL
				"f.id,"
				"f.name,"
				DISTINCT_F2P_DB_AND_POST_IDS ","
				DISTINCT_F2T_TAG_IDS
			"FROM _file f "
			"LEFT JOIN file2tag f2t ON f2t.file_id=f.id "
			JOIN_FILE_THUMBNAIL
			"LEFT JOIN file2qt5md5 f2h ON f2h.file=f.id "
			"LEFT JOIN file2post f2p ON f2p.file=f.id "
			"WHERE f.id IN ("
				"SELECT file_id "
				"FROM file2tag "
				"WHERE tag_id=", id,
			")"
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			"GROUP BY f.id "
			"LIMIT 100"
		);
		
		this->asciify_file_info();
		
#ifdef n_cached
		this->add_buf_to_cache(cached_stuff::files_given_tag, id);
#endif
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view files_given_ids(const char* s){
		const char* const file_ids  = get_comma_separated_ints(&s, ' ');
		if (file_ids == nullptr)
			return _r::not_found;
		const size_t file_ids_len  = (uintptr_t)s - (uintptr_t)file_ids;
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
		this->mysql_query(
			"SELECT "
				FILE_THUMBNAIL
				"f.id,"
				"f.name,"
				DISTINCT_F2P_DB_AND_POST_IDS ","
				DISTINCT_F2T_TAG_IDS
			"FROM _file f "
			"LEFT JOIN file2tag f2t ON f2t.file_id=f.id "
			JOIN_FILE_THUMBNAIL
			"LEFT JOIN file2qt5md5 f2h ON f2h.file=f.id "
			"LEFT JOIN file2post f2p ON f2p.file=f.id "
			"WHERE f.id IN (", _f::strlen, file_ids, file_ids_len, ")"
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			"GROUP BY f.id "
			"LIMIT 100"
		);
		
		this->asciify_file_info();
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view files_given_dir(const char* id_str){
		const uint64_t id = a2n<uint64_t>(id_str);
		
#ifdef n_cached
		if (const int indx = cached_stuff::from_cache(cached_stuff::files_given_dir, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
#endif
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(id_str, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
		this->mysql_query(
			"SELECT "
				FILE_THUMBNAIL
				"f.id,"
				"f.name,"
				DISTINCT_F2P_DB_AND_POST_IDS ","
				DISTINCT_F2T_TAG_IDS
			"FROM _file f "
			"LEFT JOIN file2tag f2t ON f2t.file_id=f.id "
			JOIN_FILE_THUMBNAIL
			"LEFT JOIN file2qt5md5 f2h ON f2h.file=f.id "
			"LEFT JOIN file2post f2p ON f2p.file=f.id "
			"WHERE f.dir=", id, " "
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			"GROUP BY f.id "
			"LIMIT 100"
		);
		
		this->asciify_file_info();
		
#ifdef n_cached
		this->add_buf_to_cache(cached_stuff::files_given_dir, id);
#endif
		
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
	
	std::string_view get_dir_json(const char* s){
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		if (user_id != user_auth::SpecialUserID::guest){
			uint64_t id;
			const char* str1;
			const char* str2;
			constexpr _r::flag::Dict dict;
			this->mysql_query(
				"SELECT id, name, device "
				"FROM _dir "
				"WHERE id NOT IN" USER_DISALLOWED_DIRS(user_id)
			);
			this->itr = this->buf;
			this->init_json(&this->itr, dict, nullptr, &id, &str1, &str2);
			return this->get_buf_as_string_view();
		}
		
		std::unique_lock lock(_r::dirs_json_mutex);
		if (unlikely(regenerate_dir_json)){
			// WARNING: Race condition since init_json uses global mysql objects
			// TODO: Eliminate race with mutex
			regenerate_dir_json = false;
			uint64_t id;
			const char* str1;
			const char* str2;
			constexpr _r::flag::Dict dict;
			this->mysql_query_buf(
				"SELECT id, name, device "
				"FROM _dir "
				"WHERE id NOT IN" USER_DISALLOWED_DIRS__COMPILE_TIME(GUEST_ID_STR)
			);
			this->init_json(nullptr, dict, &_r::dirs_json, &id, &str1, &str2);
		}
		return _r::dirs_json;
	}
	
	std::string_view get_device_json(const char* s){
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
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
	
	std::string_view get_protocol_json(const char* s){
		this->mysql_query_buf("SELECT id, name FROM protocol");
		
		std::unique_lock lock(_r::protocol_json_mutex);
		if (unlikely(regenerate_protocol_json)){
			regenerate_protocol_json = false;
			uint64_t id; // unsigned, really - just can't justify creating another function for template
			const char* name;
			constexpr _r::flag::Dict dict;
			this->init_json(nullptr, dict, &_r::protocol_json, &id, &name);
		}
		return _r::protocol_json;
	}
	
	std::string_view get_tag_json(const char* s){
		#define get_tag_json_qry_prefix \
			"SELECT "\
				"t.id,"\
				"t.name,"\
				"GROUP_CONCAT(p.thumbnail ORDER BY (1/(1+t2pt.depth))*(p.thumbnail!=\"\") DESC LIMIT 1),"\
				"GROUP_CONCAT(p.cover     ORDER BY (1/(1+t2pt.depth))*(p.cover    !=\"\") DESC LIMIT 1) "\
			"FROM _tag t "\
			"JOIN tag2parent_tree t2pt ON t2pt.tag=t.id "\
			"JOIN _tag p ON p.id=t2pt.parent "\
			"WHERE (t2pt.depth=0 OR p.thumbnail != \"\" OR p.cover != \"\")"\
			  "AND t.id NOT IN"
		#define get_tag_json_qry_postfix \
			  "AND p.id NOT IN"  
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		if (user_id != user_auth::SpecialUserID::guest){
			uint64_t id;
			const char* name;
			const char* str1;
			const char* str2;
			constexpr _r::flag::Dict dict;
			this->mysql_query(
				get_tag_json_qry_prefix
				USER_DISALLOWED_TAGS(user_id)
				get_tag_json_qry_postfix
				USER_DISALLOWED_TAGS(user_id)
				"GROUP BY t.id"
			);
			this->itr = this->buf;
			this->init_json(&this->itr, dict, nullptr, &id, &name, &str1, &str2);
			return this->get_buf_as_string_view();
		}
		
		std::unique_lock lock(_r::tags_json_mutex);
		if (unlikely(regenerate_tag_json)){
			regenerate_tag_json = false;
			uint64_t id;
			const char* name;
			const char* str1;
			const char* str2;
			constexpr _r::flag::Dict dict;
			this->mysql_query_buf(
				get_tag_json_qry_prefix
				USER_DISALLOWED_TAGS__COMPILE_TIME(GUEST_ID_STR)
				get_tag_json_qry_postfix
				USER_DISALLOWED_TAGS__COMPILE_TIME(GUEST_ID_STR)
				"GROUP BY t.id"
			);
			this->init_json(nullptr, dict, &_r::tags_json, &id, &name, &str1, &str2);
		}
		return _r::tags_json;
	}
	
	std::string_view get_tag2parent_json(const char* s){
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		if (user_id != user_auth::SpecialUserID::guest){
			uint64_t id;
			uint64_t id2;
			constexpr _r::flag::Arr arr;
			this->mysql_query(
				"SELECT tag_id, parent_id "
				"FROM tag2parent t2p "
				"WHERE t2p.tag_id NOT IN" USER_DISALLOWED_TAGS(user_id)
				  "AND t2p.parent_id NOT IN" USER_DISALLOWED_TAGS(user_id)
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
				"SELECT tag_id, parent_id "
				"FROM tag2parent t2p "
				"WHERE t2p.tag_id NOT IN" USER_DISALLOWED_TAGS__COMPILE_TIME(GUEST_ID_STR)
				  "AND t2p.parent_id NOT IN" USER_DISALLOWED_TAGS__COMPILE_TIME(GUEST_ID_STR)
			);
			this->init_json(nullptr, arr, &_r::tag2parent_json, &id, &id2);
		}
		return _r::tag2parent_json;
	}
	
	constexpr
	std::string_view return_api(const char* s){
		switch(*(s++)){
			case 'd':
				switch(*(s++)){
					case '.':
						// /a/d.json
						return this->get_dir_json(s);
					case '/':
						switch(*(s++)){
							case 'i':
								switch(*(s++)){
									case '/':
										// /a/d/i/
										return dir_info(s);
								}
								break;
						}
						break;
				}
				break;
			case 'D':
				switch(*(s++)){
					case '.':
						// D.json
						return this->get_device_json(s);
				}
				break;
			case 'P':
				switch(*(s++)){
					case '.':
						// /a/P.json
						return this->get_protocol_json(s);
				}
				break;
			case 'f':
				switch(*(s++)){
					case '/':
						switch(*(s++)){
							case 'i':
								switch(*(s++)){
									case 'd':
										switch(*(s++)){
											case '/':
												// /f/id/123,456,789
												return this->files_given_ids(s);
										}
										break;
									case '/':
										// /a/f/i/
										return file_info(s);
								}
								break;
							case 'd':
								switch(*(s++)){
									case '/':
										// /a/f/d/
										return files_given_dir(s);
								}
								break;
							case 't':
								switch(*(s++)){
									case '/':
										// /a/f/t/
										return files_given_tag(s);
								}
								break;
						}
						break;
				}
				break;
			case 't':
				switch(*(s++)){
					case '.':
						// t.json
						return this->get_tag_json(s);
					case '2':
						// /a/t2p.json
						return this->get_tag2parent_json(s);
					case '/':
						switch(*(s++)){
							case 'f':
								switch(*(s++)){
									case '/':
										return tags_given_file(s);
								}
								break;
						}
						break;
				}
				break;
			case 'm':
				switch(*(s++)){
					case 't':
						switch(*(s++)){
							case '.':
								// mt.json
								return this->get_mimetype_json(s);
						}
						break;
				}
				break;
			case 'x':
				switch(*(s++)){
					case '.':
						// /a/x.json
						return _r::external_db_json;
					case '/':
						switch(*(s++)){
							case 'u':
								switch(*(s++)){
									case '/':
										switch(*(s++)){
											case 'p':
												switch(*(s++)){
													case '/':
														switch(*(s++)){
															case 'c':
																switch(*(s++)){
																	case '/':
																		// /a/x/u/p/c/DB_ID/USER_ID
																		// Commented-on posts
																		return this->external_user_posts(s, DatabaseInfo::has_cmnt_tbl, "cmnt", "post");
																}
																break;
															case 'l':
																switch(*(s++)){
																	case '/':
																		// /a/x/u/p/l/DB_ID/USER_ID
																		return this->external_user_posts(s, DatabaseInfo::has_post2like_tbl, "post2like", "post");
																}
																break;
															case 'u':
																switch(*(s++)){
																	case '/':
																		// /a/x/u/p/u/DB_ID/USER_ID
																		return this->external_user_posts(s, DatabaseInfo::has_post_tbl, "post", "id");
																}
																break;
														}
														break;
												}
												break;
											case 'i':
												switch(*(s++)){
													case '/':
														// /a/x/u/i/DB_ID/USER_ID
														return this->external_user_info(s);
												}
												break;
										}
										break;
								}
								break;
							case 'p':
								switch(*(s++)){
									case '/':
										switch(*(s++)){
											case 'i':
												switch(*(s++)){
													case '/':
														// /a/x/p/i/DB_ID/POST_ID
														return this->external_post_info(s);
												}
												break;
											case 'l':
												switch(*(s++)){
													case '/':
														// /a/x/p/l/DB_ID/POST_ID
														return this->external_post_likes(s);
												}
												break;
										}
										break;
								}
								break;
						}
						break;
				}
				break;
		}
		return _r::not_found;
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
		constexpr static const size_t stream_block_sz = 1024 * 500; // WARNING: Will randomly truncate responses, usually around several MiBs // TODO: Increase this buffer size.
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
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
		this->mysql_query(
			"SELECT m.name, CONCAT(d.name, f", (dir_id==0)?"":"2", ".name) "
			"FROM _file f ",
			(dir_id==0)?"":"JOIN file_backup f2 ON f2.file=f.id ",
			"JOIN _dir d ON d.id=f", (dir_id==0)?"":"2", ".dir "
			"JOIN mimetype m ON m.id=f.mimetype "
			"WHERE f.id=", id, " "
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			  DIR_TBL_USER_PERMISSION_FILTER(user_id)
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
	
	FunctionSuccessness dl_file(const UserIDIntType user_id,  const uint64_t dir_id,  const char* const file_name,  const char* const url,  const bool overwrite_existing){
		FunctionSuccessness rc = FunctionSuccessness::ok;
		static char dst_pth[4096];
		
		const char* dir_name = nullptr;
		this->mysql_query("SELECT name FROM _dir WHERE id=", dir_id, " AND id NOT IN " USER_DISALLOWED_DIRS(user_id));
		if (not this->mysql_assign_next_row(&dir_name)){
			// No visible directory with the requested ID
			// MySQL results already freed
			return FunctionSuccessness::malicious_request;
		}
		
		printf("dl_file %s %lu %s\n", (overwrite_existing)?">":"+", dir_id, url);
		
		compsky::asciify::asciify(dst_pth, dir_name, file_name, '\0');
		
		rc = dl_file__curl(url, dst_pth, overwrite_existing);
		
		dl_file__cleanup:
		this->mysql_free_res();
		return rc;
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
			"(tag_id, parent_id, user)"
			"SELECT t.id, p.id,", user_id, " "
			"FROM _tag t "
			"JOIN _tag p "
			"WHERE p.id IN (", _f::strlen, parent_ids, parent_ids_len, ") "
			  "AND t.name=\"", _f::esc, '"', _f::strlen, tag_name_len,  tag_name, "\" "
			  "AND t.id NOT IN " USER_DISALLOWED_TAGS(user_id)
			  "AND p.id NOT IN " USER_DISALLOWED_TAGS(user_id)
			"ON DUPLICATE KEY UPDATE tag_id=tag_id"
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
		regenerate_tag_json = true;
	}
	
	void add_D_to_db(const UserIDIntType user_id,  const unsigned protocol,  const char* const url,  const size_t url_len){
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
	}
	
	void add_d_to_db(const UserIDIntType user_id,  const uint64_t device,  const char* const url,  const size_t url_len){
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
	}
	
	bool add_f_to_db(const UserIDIntType user_id,  const char* const tag_ids,  const size_t tag_ids_len,  const char* url,  const size_t dir_id_and_url_len){
		const char* const dir_id_and_url = url;
		const uint64_t dir_id = a2n<uint64_t>(&url);
		if(unlikely(*url != '\t'))
			return true;
		++url;
		const size_t url_len = dir_id_and_url_len - ( (uintptr_t)url - (uintptr_t)dir_id_and_url );
		
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
			"(file_id, tag_id, user)"
			"SELECT f.id, t.id,", user_id, " "
			"FROM _file f "
			"JOIN _dir d ON d.id=f.dir "
			"JOIN _tag t "
			"WHERE t.id IN (", _f::strlen, tag_ids, tag_ids_len, ") "
			  "AND f.name=SUBSTR(\"", _f::esc, '"', _f::strlen, url_len, url, "\",LENGTH(d.name)+1) "
			  "AND f.dir=", dir_id, " "
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			  DIR_TBL_USER_PERMISSION_FILTER(user_id)
			"ON DUPLICATE KEY UPDATE file_id=file_id"
		);
		
		return false;
	}
	
	std::string_view add_to_tbl(const char tbl,  const char* s){
		const char* tag_ids;
		size_t tag_ids_len;
		
		if((tbl == 'f') or (tbl == 't')){
			tag_ids = get_comma_separated_ints(&s, '/');
			if (tag_ids == nullptr)
				return _r::not_found;
			tag_ids_len = (uintptr_t)s - (uintptr_t)tag_ids;
			++s; // Skip trailing slash
		}
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
		uint64_t parent_id;
		if ((tbl != 'f') and (tbl != 't')){
			parent_id = a2n<uint64_t>(&s);
			// device for tbl d, protocol (unsigned) for tbl D
		}
		
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
					this->add_d_to_db(user_id, parent_id, url, url_len);
					break;
				case 'D':
					this->add_D_to_db(user_id, parent_id, url, url_len);
					break;
				case 't':
					this->add_t_to_db(user_id, tag_ids, tag_ids_len, url, url_len);
					break;
			}
			if (*s == 0)
				return _r::post_ok;
		} while(true);
	}
	
	std::string_view post__dl(const char* s){
		static char url_buf[4096];
		
		const uint64_t dir_id = a2n<uint64_t>(&s);
		++s;
		
		const char* const tag_ids  = get_comma_separated_ints(&s, '/');
		if (tag_ids == nullptr)
			return _r::not_found;
		const size_t tag_ids_len  = (uintptr_t)s - (uintptr_t)tag_ids;
		++s; // Skip slash
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
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
					
					switch(dl_file(user_id, dir_id, file_name, url_buf, is_html_file)){
						case FunctionSuccessness::server_error:
							++n_errors;
						case FunctionSuccessness::ok:
							break;
						case FunctionSuccessness::malicious_request:
							return _r::not_found;
					}
					
					this->mysql_exec(
						"INSERT INTO _file"
						"(name, dir, user)"
						"VALUES(",
							'"', _f::esc, '"', file_name, '"', ',',
							dir_id, ',',
							user_id,
						")"
						"ON DUPLICATE KEY UPDATE dir=dir"
					);
					this->mysql_exec(
						"INSERT INTO file2tag"
						"(file_id, tag_id, user)"
						"SELECT f.id, t.id,", user_id, " "
						"FROM _file f "
						"JOIN _tag t "
						"WHERE f.name=\"", _f::esc, '"', file_name, "\" "
						  "AND f.dir=", dir_id, " "
						  "AND t.id IN (", _f::strlen, tag_ids, tag_ids_len,") "
						  FILE_TBL_USER_PERMISSION_FILTER(user_id)
						  TAG_TBL_USER_PERMISSION_FILTER(user_id)
						"ON DUPLICATE KEY UPDATE file_id=file_id"
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
	
	void tag_parentisation(const UserIDIntType user_id,  const char* const child_ids,  const char* const tag_ids,  const size_t child_ids_len,  const size_t tag_ids_len){
		this->mysql_exec(
			"INSERT INTO tag2parent (tag_id, parent_id, user) "
			"SELECT t.id, p.id,", user_id, " "
			"FROM _tag t "
			"JOIN _tag p "
			"WHERE t.id IN (", _f::strlen, child_ids, child_ids_len, ")"
			  "AND p.id IN (", _f::strlen, tag_ids,   tag_ids_len,   ")"
			  "AND t.id NOT IN" USER_DISALLOWED_TAGS(user_id)
			  "AND p.id NOT IN" USER_DISALLOWED_TAGS(user_id)
			"ON DUPLICATE KEY UPDATE tag_id=tag_id"
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
		
		regenerate_tag_json = true;
		regenerate_tag2parent_json = true;
	}
	
	std::string_view post__add_parents_to_tags(const char* s){
		const char* const tag_ids = get_comma_separated_ints(&s, '/');
		if (tag_ids == nullptr)
			return _r::not_found;
		const size_t tag_ids_len = (uintptr_t)s - (uintptr_t)tag_ids;
		++s; // Skip trailing slash
		
		const char* const parent_ids  = get_comma_separated_ints(&s, ' ');
		if (parent_ids == nullptr)
			return _r::not_found;
		const size_t parent_ids_len  = (uintptr_t)s - (uintptr_t)parent_ids;
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
		this->tag_parentisation(user_id, tag_ids, parent_ids, tag_ids_len, parent_ids_len);
		
		return _r::post_ok;
	}
	
	std::string_view post__add_children_to_tags(const char* s){
		const char* const tag_ids = get_comma_separated_ints(&s, '/');
		if (tag_ids == nullptr)
			return _r::not_found;
		const size_t tag_ids_len = (uintptr_t)s - (uintptr_t)tag_ids;
		++s; // Skip trailing slash
		
		const char* const child_ids  = get_comma_separated_ints(&s, ' ');
		if (child_ids == nullptr)
			return _r::not_found;
		const size_t child_ids_len  = (uintptr_t)s - (uintptr_t)child_ids;
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
		this->tag_parentisation(user_id, child_ids, tag_ids, child_ids_len, tag_ids_len);
		
		return _r::post_ok;
	}
	
	std::string_view post__add_tag_to_file(const char* s){
		const char* const file_ids = get_comma_separated_ints(&s, '/');
		if (file_ids == 0)
			return _r::not_found;
		const size_t file_ids_len = (uintptr_t)s - (uintptr_t)file_ids;
		++s; // Skip trailing slash
		const char* const tag_ids  = get_comma_separated_ints(&s, ' ');
		if (tag_ids == 0)
			return _r::not_found;
		const size_t tag_ids_len  = (uintptr_t)s - (uintptr_t)tag_ids;
		
		const UserIDIntType user_id = user_auth::get_user_id(get_cookie(s, "username="));
		if (user_id == user_auth::SpecialUserID::invalid)
			return _r::not_found;
		
		this->mysql_exec(
			"INSERT INTO file2tag (tag_id, file_id, user) "
			"SELECT t.id,f.id,", user_id, " "
			"FROM _tag t "
			"JOIN _file f "
			"WHERE t.id IN (", _f::strlen, tag_ids,  tag_ids_len,  ")"
			  "AND f.id IN (", _f::strlen, file_ids, file_ids_len, ")"
			  FILE_TBL_USER_PERMISSION_FILTER(user_id)
			  TAG_TBL_USER_PERMISSION_FILTER(user_id)
			"ON DUPLICATE KEY UPDATE file_id=file_id"
		);
		
		return _r::post_ok;
	}
	
	std::string_view post__add_var_to_file(const char* s){
		
		return _r::post_ok;
	}
	
	std::string_view post__edit_tag_cmnt(const char* s){
		const uint64_t tag_id = a2n<uint64_t>(&s);
		++s;
		
		printf("Edit tag cmnt: %lu: %s\n", tag_id, s);
		
		return _r::post_ok;
	}
	
	constexpr
	std::string_view determine_response(const char* s){
		switch(which_method(s)){
			case _method::GET:
				switch(*(s++)){
					case '/':
						switch(*(s++)){
							case ' ':
								return
									#include "headers/return_code/OK.c"
									#include "headers/Content-Type/html.c"
									#include "headers/Cache-Control/1day.c"
									"\n"
									#include "html/profile.html"
								;
							case 'a':
								switch(*(s++)){
									case '/':
										return this->return_api(s);
								}
								break;
							case 'i':
								switch(*(s++)){
									case '/':
										// /i/
										return this->return_img(s);
								}
								break;
							case 'f':
								switch(*(s++)){
									case 'a':
										switch(*(s++)){
											case 'v':
												// /favicon.ico
												return std::string_view(
													#include "headers/return_code/OK.c"
													#include "headers/Content-Type/ico.c"
													#include "headers/Cache-Control/1day.c"
													"Content-Length: 318\n"
													"\n"
													#include "favicon.txt"
													, std::char_traits<char>::length(
														#include "headers/return_code/OK.c"
														#include "headers/Content-Type/ico.c"
														#include "headers/Cache-Control/1day.c"
														"Content-Length: 318\n"
														"\n"
													) + 318
												);
										}
										break;
								}
							case 's':
								switch(*(s++)){
									case '/':
										return _r::return_static(s);
								}
								break;
							case 'S':
								switch(*(s++)){
									case '/':
										switch(*(s++)){
											case 'f':
												switch(*(s++)){
													case '/':
														// /S/f/
														return this->stream_file(s);
												}
												break;
										}
										break;
								}
								break;
						}
						break;
				}
				break;
			case _method::POST:
				switch(*(s++)){
					case '/':
						switch(*(s++)){
							case 'a':
								switch(*(s++)){
									case 'd':
										switch(*(s++)){
											case 'd':
												switch(*(s++)){
													case '-':
														switch(*(s++)){
															case 'D':
																switch(*(s++)){
																	case '/':
																		// /a/add-D/
																		return this->add_to_tbl('D', s);
																}
																break;
															case 'd':
																switch(*(s++)){
																	case '/':
																		// /a/add-d/
																		return this->add_to_tbl('d', s);
																}
																break;
															case 'f':
																switch(*(s++)){
																	case '/':
																		// /a/add-f/
																		return this->add_to_tbl('f', s);
																}
																break;
															case 't':
																switch(*(s++)){
																	case '/':
																		// /a/add-t/
																		return this->add_to_tbl('t', s);
																}
																break;
														}
														break;
												}
												break;
										}
										break;
								}
								break;
							case 'x':
								switch(*(s++)){
									case '/':
										switch(*(s++)){
											case 'c':
												switch(*(s++)){
													case '/':
														switch(*(s++)){
															case 'r':
																switch(*(s++)){
																	case 'm':
																		switch(*(s++)){
																			case '/':
																				// /x/c/rm/DB_ID/CMNT_ID
																				return this->external_cmnt_rm(s);
																		}
																		break;
																}
																break;
														}
														break;
												}
												break;
										}
										break;
								}
								break;
							case 'd':
								switch(*(s++)){
									case 'l':
										switch(*(s++)){
											case '/':
												// /dl/
												return this->post__dl(s);
										}
										break;
								}
								break;
							case 't':
								switch(*(s++)){
									case '/':
										switch(*(s++)){
											case 'c':
												switch(*(s++)){
													case '/':
														// /t/c/ i.e. /t/child/
														return this->post__add_children_to_tags(s);
												}
												break;
											case 'p':
												switch(*(s++)){
													case '/':
														// /t/c/ i.e. /t/child/
														return this->post__add_parents_to_tags(s);
												}
												break;
										}
										break;
								}
								break;
							case 'f':
								switch(*(s++)){
									case '/':
										switch(*(s++)){
											case 'o':
												switch(*(s++)){
													case 'r':
														switch(*(s++)){
															case 'i':
																switch(*(s++)){
																	case 'g':
																		switch(*(s++)){
																			case '/':
																				// /f/orig/FILE_ID/DIR_ID
																				return this->replace_file_path_and_set_old_path_as_backup(s);
																		}
																		break;
																}
																break;
														}
														break;
												}
												break;
											case 'v':
												switch(*(s++)){
													case '/':
														// /f/v/
														return this->post__add_var_to_file(s);
												}
												break;
											case 't':
												switch(*(s++)){
													case '/':
														// /f/t/
														return this->post__add_tag_to_file(s);
												}
												break;
										}
										break;
								}
								break;
							case 'q':
								switch(*(s++)){
									case '/':
										return this->parse_qry(s);
								}
								break;
							case 'l':
								switch(*(s++)){
									case 'o':
										switch(*(s++)){
											case 'g':
												switch(*(s++)){
													case 'i':
														switch(*(s++)){
															case 'n':
																switch(*(s++)){
																	case ' ':
																		// /login
																		break;
																}
																break;
														}
														break;
												}
												break;
										}
										break;
								}
								break;
						}
						break;
				}
				break;
		}
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
	
		void read(Context* ctx,  const char* const msg) override {
			this->reset_buf_index();
			for(const char* msg_itr = msg;  *msg_itr != 0  &&  *msg_itr != '\n';  ++msg_itr){
				this->asciify(*msg_itr);
			}
			*this->itr = 0;
			const std::string client_addr = ctx->getPipeline()->getTransportInfo()->remoteAddr->getHostStr();
			std::cout << client_addr << '\t' << this->buf << std::endl;
			const std::string_view v = likely(std::find(banned_client_addrs.begin(), banned_client_addrs.end(), client_addr) == banned_client_addrs.end()) ? this->determine_response(msg) : _r::banned_client;
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
			case 'x':
				external_db_env_vars.push_back(*(++argv));
				break;
			default:
				goto help;
		}
	}
	
	if (port_n == 0){
		help:
		fprintf(
			stderr,
			help::text
		);
		return 1;
	}
	
	int dummy_argc = 0;
	folly::Init init(&dummy_argc, &dummy_argv);
	
	if (mysql_library_init(0, NULL, NULL))
		throw compsky::mysql::except::SQLLibraryInit();
	
	db_infos.reserve(external_db_env_vars.size());
	std::string db_name2id_json = "[\"";
	MYSQL_RES* res;
	MYSQL_ROW row;
	for (unsigned i = 0;  i < external_db_env_vars.size();  ++i){
		char* const db_env_name = external_db_env_vars.at(i);
		
		const DatabaseInfo& db_info = db_infos.emplace_back(db_env_name, (i!=0));
		db_name2id_json += db_info.name();
		db_name2id_json += "\",\"";
		
		if (i == 0)
			continue;
		
		char buf[200];
		compsky::mysql::query(db_infos.at(0).mysql_obj, res, buf, "SELECT id FROM external_db WHERE name=\"", db_info.name(), "\"");
		unsigned id = 0;
		while(compsky::mysql::assign_next_row(res, &row, &id));
		if (id == 0){
			fprintf(stderr,  "External database not recorded in external_db table: %s\n", db_info.name());
			return 1;
		}
		db_indx2id[i] = id;
	}
	db_name2id_json.pop_back();   // Remove trailing quotation mark
	db_name2id_json.back() = ']'; // Replace trailing comma
	_r::external_db_json = db_name2id_json.c_str();
	
	printf("_r::external_db_json set\n");
	
	compsky::mysql::query_buffer(db_infos.at(0).mysql_obj, res, "SELECT id, name FROM user");
	user_auth::users.reserve(compsky::mysql::n_results<size_t>(res));
	UserIDIntType id;
	const char* name;
	while(compsky::mysql::assign_next_row__no_free(res, &row, &id, &name))
		user_auth::users.emplace_back(name, id);
	
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
