#include "FrameDecoder.h"
#include "CStringCodec.h"

#include <compsky/mysql/query.hpp>
#include <compsky/asciify/asciify.hpp>

#include <folly/init/Init.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>

#include <mutex>
#include <cstring> // for malloc

#define FILE_THUMBNAIL "IFNULL(IFNULL(f2tn.x, CONCAT('/i/f/', LOWER(HEX(f2h.x)))), \"\"),"
#define JOIN_FILE_THUMBNAIL "LEFT JOIN file2thumbnail f2tn ON f2tn.file=f.id "

#include <curl/curl.h>


#ifndef n_cached
# error "Please define -Dn_cached=<NUMBER_OF_ITEMS_TO_CACHE>"
#endif


typedef wangle::Pipeline<folly::IOBufQueue&,  const char*> RTaggerPipeline;

namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
	constexpr static const compsky::asciify::flag::AlphaNumeric alphanum;
	constexpr static const compsky::asciify::flag::StrLen strlen;
	//constexpr static const compsky::asciify::flag::MaxBufferSize max_sz;
}

namespace _mysql {
	MYSQL* mysql_obj;
	char buf[512];
	char* auth[6];
	constexpr static const size_t buf_sz = 512; // TODO: Alloc file size
}

const char* CACHE_DIR = nullptr;
size_t CACHE_DIR_STRLEN;

enum FunctionSuccessness {
	ok,
	malicious_request,
	server_error,
	unimplemented,
	COUNT
};

namespace cached_stuff {
	// WARNING: This is only for functions whose results are guaranteed to be shorter than the max_buf_len.
	constexpr static const size_t max_buf_len = 1  +  100 * (1 + 20 + 1 + 2*64 + 1 + 20 + 1 + 2*20 + 3 + 2*20 + 1 + 1 + 1)  +  1  +  1; // == 25803
	static char cache[n_cached * max_buf_len];
	enum {
		files_given_tag,
		files_given_dir,
		tags_given_file,
		dir_info,
		file_info,
		n_fns
	};
	struct ID {
		unsigned int n_requests;
		unsigned int which_cached_fn;
		uint64_t user_id;
		size_t sz;
	};
	static ID cached_IDs[n_cached] = {}; // Initialise to zero
	
	int from_cache(const unsigned int which_cached_fn,  const uint64_t user_id){
		int i = 0;
		while (i < n_cached){
			ID& id = cached_IDs[i];
			++i;
			if ((id.which_cached_fn == which_cached_fn) and (id.user_id == user_id)){
				++id.n_requests;
				return i;
			}
		}
		return 0;
	}
}

std::vector<std::string> banned_client_addrs;

constexpr
bool is_valid_hex_char(const char c){
	switch(c){
		case '0' ... '9':
		case 'a' ... 'f':
			return true;
		default:
			return false;
	}
}

template<typename Int>
Int a2n(const char* s){
	uint64_t n = 0;
	while(*s >= '0'  &&  *s <= '9'){
		n *= 10;
		n += *s - '0';
		++s;
	}
	return n;
}

template<typename Int>
Int a2n(const char** s){
	uint64_t n = 0;
	while(**s >= '0'  &&  **s <= '9'){
		n *= 10;
		n += **s - '0';
		++(*s);
	}
	return n;
}

constexpr
bool is_integer(const char c){
	return ((c >= '0')  and  (c <= '9'));
}

constexpr
const char* skip_to(const char* s,  const char c){
	while(true){
		if (unlikely(*s == 0))
			return nullptr;
		if (*s == c)
			return s;
	}
}

constexpr
const char* get_comma_separated_ints(const char** str,  const char separator){
	const char* const start = *str;
	while(true){
		if (not is_integer(**str))
			return nullptr;
		do {
			++(*str);
		} while (is_integer(**str));
		
		if (**str == ','){
			++(*str);
			continue;
		}
		
		return (**str == separator) ? start : nullptr;
	}
}

constexpr
bool get_range(const char* headers,  size_t& from,  size_t& to){
	while(*(++headers) != 0){ // NOTE: headers is guaranteed to be more than 0 characters long, as we have already guaranteed that it starts with the file id
		if (*headers != '\n')
			continue;
		bool is_range_header = false;
		switch(*(++headers)){
			case 'R':
				switch(*(++headers)){
					case 'a':
						switch(*(++headers)){
							case 'n':
								switch(*(++headers)){
									case 'g':
										switch(*(++headers)){
											case 'e':
												switch(*(++headers)){
													case ':':
														switch(*(++headers)){
															case ' ':
																switch(*(++headers)){
																	case 'b':
																		switch(*(++headers)){
																			case 'y':
																				switch(*(++headers)){
																					case 't':
																						switch(*(++headers)){
																							case 'e':
																								switch(*(++headers)){
																									case 's':
																										switch(*(++headers)){
																											case '=':
																												is_range_header = true;
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
		if (*headers == 0)
			break;
		if (likely(not is_range_header))
			continue;
		
		++headers; // Skip '=' - a2n is safe even if the next character is null
		from = a2n<size_t>(&headers);
		
		if (*headers != '-')
			return true;
		
		++headers; // Skip '-' - a2n is safe even if the next character is null
		to = a2n<size_t>(headers);
		
		return false;
	}
	from = 0;
	to = 0;
	return false;
}

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
	constexpr static const std::string_view not_found =
		#include "headers/return_code/NOT_FOUND.c"
		"\n"
		"Not Found"
	;
	
	constexpr static const std::string_view invalid_file = 
		#include "headers/return_code/NOT_FOUND.c"
		"\n"
		"File does not exist"
	;
	
	constexpr static const std::string_view server_error =
		#include "headers/return_code/SERVER_ERROR.c"
		"\n"
		"Server error"
	;
	
	constexpr static const std::string_view post_ok =
		#include "headers/return_code/OK.c"
		"\n"
		"OK"
	;
	
	/*
	constexpr static const std::string_view bad_request =
		#include "headers/return_code/BAD_REQUEST.c"
		"\n"
		"Bad Request"
	;
	*/
	constexpr static const std::string_view bad_request = not_found;
	
	constexpr static const std::string_view banned_client =
		#include "headers/return_code/UNAUTHORISED.c"
		"\n"
		"Your IP address has been temporarily banned"
	;
	
	constexpr static const std::string_view img_not_found(
		#include "headers/return_code/OK.c" // To encourage browsers to cache it.
		#include "headers/Content-Type/png.c"
		#include "headers/Cache-Control/1day.c"
		"Content-Length: 195\n"
		"\n"
		#include "i/404.txt" // WARNING: Corrupt, because the end of transmission character is skipped in this 'raw' string. // TODO: Figure out why
		, std::char_traits<char>::length(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/png.c"
			#include "headers/Cache-Control/1day.c"
			"Content-Length: 195\n"
			"\n"
		) + 195
	);
	
	constexpr static const std::string_view EMPTY_JSON_LIST = 
		#include "headers/return_code/OK.c" // To encourage browsers to cache it.
		#include "headers/Content-Type/json.c"
		#include "headers/Cache-Control/1day.c"
		"Content-Length: 2\n"
		"\n"
		"[]"
	;
	
	constexpr
	std::string_view return_static(const char* s){
		switch(*(s++)){
			case 'a':
				// a.css
				return
					#include "headers/return_code/OK.c"
					#include "headers/Content-Type/css.c"
					#include "headers/Cache-Control/1day.c"
					"\n"
					#include "static/all.css"
				;
			case 't':
				switch(*(s++)){
					case 'b':
						switch(*(s++)){
							case 'l':
								switch(*(s++)){
									case '1':
										// tbl1.css
										return
											#include "headers/return_code/OK.c"
											#include "headers/Content-Type/css.c"
											#include "headers/Cache-Control/1day.c"
											"\n"
											#include "static/table_as_blocks.css"
										;
									case '2':
										// tbl2.css
										return
											#include "headers/return_code/OK.c"
											#include "headers/Content-Type/css.c"
											#include "headers/Cache-Control/1day.c"
											"\n"
											#include "static/table_as_table.css"
										;
								}
								break;
						}
						break;
				}
				break;
			case 'u':
				switch(*(s++)){
					case '.':
						switch(*(s++)){
							case 'j':
								switch(*(s++)){
									case 's':
										switch(*(s++)){
											case ' ':
												return
													#include "headers/return_code/OK.c"
													#include "headers/Content-Type/javascript.c"
													#include "headers/Cache-Control/1day.c"
													"\n"
													#include "static/utils.js"
												;
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
	
	static char buf[1024 * 1024]; // Needs to be large enough for file thumbnails
	char* itr = nullptr;
	
	static const char* tags_json;
	static const char* tag2parent_json;
	static const char* dirs_json;
	static const char* protocols_json;
	static const char* devices_json;
	
	
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
	
	template<typename FlagType,  typename... Args>
	void init_json(const FlagType& _flag,  const char* const qry,  const char*& dst,  Args... args){
		MYSQL_RES* mysql_res;
		MYSQL_ROW mysql_row;
		
		compsky::mysql::query(
			_mysql::mysql_obj,
			mysql_res,
			buf,
			qry
		);
		
		constexpr static const char* const _headers =
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			#include "headers/Cache-Control/1day.c"
			"\n"
		;
		
		size_t sz = 0;
		
		sz += std::char_traits<char>::length(_headers);
		sz += 1;
		while(compsky::mysql::assign_next_row__no_free(mysql_res, &mysql_row, args...)){
			sz += strlens(_flag, args...);;
		}
		sz += 1;
		sz += 1;
		
		char* itr = (char*)malloc(sz);
		dst = const_cast<char*>(itr);
		if(unlikely(itr == nullptr))
			exit(4096);
		
		compsky::asciify::asciify(itr, _headers);
		compsky::asciify::asciify(itr, opener(_flag));
		mysql_data_seek(mysql_res, 0); // Reset to first result
		while(compsky::mysql::assign_next_row(mysql_res, &mysql_row, args...)){
			asciifiis(_flag, itr, args...);
		}
		if (unlikely(*(itr - 1) == ','))
			// If there was at least one iteration of the loop...
			--itr; // ...wherein a trailing comma was left
		*(itr++) = closer(_flag);
		*itr = 0;
	}
}

namespace _method {
	enum {
		GET,
		POST,
		UNKNOWN
	};
}

constexpr
unsigned int which_method(const char*& s){
	switch(*(s++)){
		case 'P':
			switch(*(s++)){
				case 'O':
					switch(*(s++)){
						case 'S':
							switch(*(s++)){
								case 'T':
									switch(*(s++)){
										case ' ':
											return _method::POST;
									}
									break;
							}
							break;
					}
					break;
			}
			break;
		case 'G':
			switch(*(s++)){
				case 'E':
					switch(*(s++)){
						case 'T':
							switch(*(s++)){
								case ' ':
									return _method::GET;
							}
							break;
					}
					break;
			}
			break;
	}
	return _method::UNKNOWN;
}

class RTaggerHandler : public wangle::HandlerAdapter<const char*,  const std::string_view> {
  private:
	constexpr static const size_t buf_sz = 100 * 1024 * 1024; // 100 MiB
	char* buf;
	char* itr;
	size_t remaining_buf_sz;
	
	static std::mutex mysql_mutex;
	MYSQL_RES* res;
	MYSQL_ROW row;
	
	bool regenerate_dir_json;
	bool regenerate_device_json;
	bool regenerate_tag_json;
	bool regenerate_tag2parent_json;
	
	constexpr
	uintptr_t buf_indx(){
		return (uintptr_t)this->itr - (uintptr_t)this->buf;
	}
	
	constexpr
	void reset_buf_index(){
		this->itr = this->buf;
		this->remaining_buf_sz = this->buf_sz;
	}
	
	inline
	char last_char_in_buf(){
		return *(this->itr - 1);
	}
	
	template<typename... Args>
	void asciify(Args... args){
		compsky::asciify::asciify(this->itr,  args...);
	};
	
	void mysql_query_using_buf(){
		this->mysql_mutex.lock();
		compsky::mysql::query_buffer(_mysql::mysql_obj, this->res, this->buf, this->buf_indx());
		this->mysql_mutex.unlock();
	}
	
	void mysql_exec_using_buf(){
		this->mysql_mutex.lock();
		compsky::mysql::exec_buffer(_mysql::mysql_obj, this->buf, this->buf_indx());
		this->mysql_mutex.unlock();
	}
	
	template<typename... Args>
	void mysql_query(Args... args){
		this->reset_buf_index();
		this->asciify(args...);
		this->mysql_query_using_buf();
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
	
	std::string_view get_buf_as_string_view(){
		return std::string_view(this->buf, this->buf_indx());
	}
	
	void add_buf_to_cache(const unsigned int which_cached_fn,  const uint64_t user_id,  const unsigned int n_requests = 1){
		using namespace cached_stuff;
		
		unsigned int min_n_requests = UINT_MAX;
		unsigned int indx = 0; // In case all IDs have n_requesets at UINT_MAX - which is extremely unlikely
		for (unsigned int i = 0;  i < n_cached;  ++i){
			const ID& id = cached_IDs[i];
			if (id.n_requests >= min_n_requests)
				continue;
			indx = i;
			min_n_requests = id.n_requests;
		}
		
		const size_t sz = this->buf_indx();
		memcpy(cache + (indx * max_buf_len),  this->buf,  sz);
		// We could alternatively re-use this->buf, and instead malloc a new buffer for this->buf - but I prefer to avoid memory fragmentation.
		cached_IDs[indx].which_cached_fn = which_cached_fn;
		cached_IDs[indx].n_requests = n_requests;
		cached_IDs[indx].user_id = user_id;
		cached_IDs[indx].sz = sz;
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
		
		if (const int indx = cached_stuff::from_cache(cached_stuff::dir_info, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
		
		this->mysql_query(
			"SELECT name "
			"FROM dir "
			"WHERE id=", id
		);
		
		const char* name;
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		);
		this->asciify('[');
		while(this->mysql_assign_next_row(&name)){
			this->asciify(
				'"', _f::esc, '"',  name, '"'
			);
		}
		this->asciify(']');
		*this->itr = 0;
		
		this->add_buf_to_cache(cached_stuff::dir_info, id);
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view file_info(const char* id_str){
		const uint64_t id = a2n<uint64_t>(id_str);
		
		if (const int indx = cached_stuff::from_cache(cached_stuff::file_info, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
		
		this->mysql_query(
			"SELECT "
				FILE_THUMBNAIL
				"f.dir,"
				"f.name,"
				"IFNULL(GROUP_CONCAT(f2t.tag_id),\"\"),"
				"mt.name "
			"FROM file f "
			"LEFT JOIN file2tag f2t ON f2t.file_id=f.id "
			"JOIN mimetype mt ON mt.id=f.mimetype "
			JOIN_FILE_THUMBNAIL
			"LEFT JOIN file2qt5md5 f2h ON f2h.file=f.id "
			"WHERE f.id=", id, " "
			"GROUP BY f.id"
		);
		
		const char* md5_hash;
		const char* dir_id;
		const char* file_name;
		const char* tag_ids;
		const char* mimetype;
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		);
		this->asciify('[');
		while(this->mysql_assign_next_row(&md5_hash, &dir_id, &file_name, &tag_ids, &mimetype)){
			this->asciify(
				'"', md5_hash, '"', ',',
				dir_id, ',',
				'"', _f::esc, '"', file_name, '"', ',',
				'"', tag_ids, '"', ',',
				'"', mimetype, '"'
			);
		}
		this->asciify(']');
		*this->itr = 0;
		
		this->add_buf_to_cache(cached_stuff::file_info, id);
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view tags_given_file(const char* id_str){
		const uint64_t id = a2n<uint64_t>(id_str);
		
		if (const int indx = cached_stuff::from_cache(cached_stuff::tags_given_file, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
		
		this->mysql_query(
			"SELECT "
				"tag_id "
			"FROM file2tag "
			"WHERE file_id=", id, " "
			"LIMIT 1000"
		);
		
		const char* tag_id;
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		);
		this->asciify('[');
		while(this->mysql_assign_next_row(&tag_id)){
			this->asciify(
				tag_id,
				','
			);
		}
		if (this->last_char_in_buf() == ',')
			// If there was at least one iteration of the loop...
			--this->itr; // ...wherein a trailing comma was left
		this->asciify(']');
		*this->itr = 0;
		
		this->add_buf_to_cache(cached_stuff::tags_given_file, id);
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view files_given_tag(const char* id_str){
		const uint64_t id = a2n<uint64_t>(id_str);
		
		if (const int indx = cached_stuff::from_cache(cached_stuff::files_given_tag, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
		
		this->mysql_query(
			"SELECT "
				FILE_THUMBNAIL
				"f.id,"
				"f.name,"
				"GROUP_CONCAT(f2t.tag_id)"
			"FROM file f "
			"JOIN file2tag f2t ON f2t.file_id=f.id "
			JOIN_FILE_THUMBNAIL
			"LEFT JOIN file2qt5md5 f2h ON f2h.file=f.id "
			"WHERE f.id IN ("
				"SELECT file_id "
				"FROM file2tag "
				"WHERE tag_id=", id,
			")"
			"GROUP BY f.id "
			"LIMIT 100"
		);
		
		//const char* protocol_id;
		const char* md5_hex;
		//const char* dir_id;
		//const char* dir_name;
		const char* f_id;
		const char* f_name;
		const char* tag_ids;
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		);
		this->asciify('[');
		while(this->mysql_assign_next_row(&md5_hex, &f_id, &f_name, &tag_ids)){
			this->asciify(
				'[',
					'"', md5_hex, '"', ',',
					//dir_id, ',',
					//'"', _f::esc, '"', dir_name, '"', ',',
					f_id, ',',
					'"', _f::esc, '"', f_name,   '"', ',',
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
		
		this->add_buf_to_cache(cached_stuff::files_given_tag, id);
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view files_given_dir(const char* id_str){
		const uint64_t id = a2n<uint64_t>(id_str);
		
		if (const int indx = cached_stuff::from_cache(cached_stuff::files_given_dir, id))
			return std::string_view(cached_stuff::cache + ((indx - 1) * cached_stuff::max_buf_len), cached_stuff::cached_IDs[indx - 1].sz);
		
		this->mysql_query(
			"SELECT "
				FILE_THUMBNAIL
				"f.id,"
				"f.name,"
				"IFNULL(GROUP_CONCAT(f2t.tag_id),\"\")"
			"FROM file f "
			"LEFT JOIN file2tag f2t ON f2t.file_id=f.id "
			JOIN_FILE_THUMBNAIL
			"LEFT JOIN file2qt5md5 f2h ON f2h.file=f.id "
			"WHERE f.dir=", id, " "
			"GROUP BY f.id "
			"LIMIT 100"
		);
		
		//const char* protocol_id;
		const char* md5_hex;
		//const char* dir_id;
		//const char* dir_name;
		const char* f_id;
		const char* f_name;
		const char* tag_ids;
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		);
		this->asciify('[');
		while(this->mysql_assign_next_row(&md5_hex, &f_id, &f_name, &tag_ids)){
			this->asciify(
				'[',
					'"', md5_hex, '"', ',',
					//dir_id, ',',
					//'"', _f::esc, '"', dir_name, '"', ',',
					f_id, ',',
					'"', _f::esc, '"', f_name,   '"', ',',
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
		
		this->add_buf_to_cache(cached_stuff::files_given_dir, id);
		
		return this->get_buf_as_string_view();
	}
	
	constexpr
	std::string_view get_dir_json(){
		if (unlikely(this->regenerate_dir_json)){
			uint64_t id;
			const char* str1;
			const char* str2;
			constexpr _r::flag::Dict dict;
			_r::init_json(dict, "SELECT id, name, device FROM dir", _r::dirs_json, &id, &str1, &str2);
		}
		return _r::dirs_json;
	}
	
	constexpr
	std::string_view get_device_json(){
		if (unlikely(this->regenerate_device_json)){
			uint64_t id;
			const char* name;
			unsigned protocol;
			const char* embed_pre;
			const char* embed_post;
			constexpr _r::flag::Dict dict;
			_r::init_json(dict, "SELECT id, name, protocol, embed_pre, embed_post FROM device", _r::devices_json, &id, &name, &protocol, &embed_pre, &embed_post);
		}
		return _r::devices_json;
	}
	
	constexpr
	std::string_view get_tag_json(){
		if (unlikely(this->regenerate_tag_json)){
			uint64_t id;
			const char* name;
			const char* str1;
			const char* str2;
			constexpr _r::flag::Dict dict;
			_r::init_json(dict,
				"SELECT "
					"t.id,"
					"t.name,"
					"GROUP_CONCAT(p.thumbnail ORDER BY (1/(1+t2pt.depth))*(p.thumbnail!=\"\") DESC LIMIT 1),"
					"GROUP_CONCAT(p.cover     ORDER BY (1/(1+t2pt.depth))*(p.cover    !=\"\") DESC LIMIT 1) "
				"FROM tag t "
				"JOIN tag2parent_tree t2pt ON t2pt.tag=t.id "
				"JOIN tag p ON p.id=t2pt.parent "
				"WHERE (t2pt.depth=0 OR p.thumbnail != \"\" OR p.cover != \"\")"
				"GROUP BY t.id"
				, _r::tags_json, &id, &name, &str1, &str2
			);
		}
		return _r::tags_json;
	}
	
	constexpr
	std::string_view get_tag2parent_json(){
		if (unlikely(this->regenerate_tag2parent_json)){
			uint64_t id;
			uint64_t id2;
			constexpr _r::flag::Arr arr;
			_r::init_json(arr,  "SELECT tag_id, parent_id FROM tag2parent", _r::tag2parent_json, &id, &id2);
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
						return this->get_dir_json();
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
						return this->get_device_json;
				}
				break;
			case 'f':
				switch(*(s++)){
					case '/':
						switch(*(s++)){
							case 'i':
								switch(*(s++)){
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
						// m.json
						return this->get_tag_json;;
					case '2':
						// /a/t2p.json
						return this->get_tag2parent_json;
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
	
	std::string_view stream_file(const char* file_id){
		constexpr static const size_t block_sz = 4096 * 1024 * 20; // ~20 MiB // WARNING: Files larger than this must be streamed in chunks, but are currently not accepted by browsers.
		constexpr static const size_t room_for_headers = 1000;
		static_assert(buf_sz  >  block_sz + room_for_headers); // 1000 is to leave room for moving headers around
		
		const uint64_t id = a2n<uint64_t>(file_id);
		
		size_t from;
		size_t to;
		if (unlikely(get_range(file_id, from, to))){
			printf("from %lu to %lu (invalid)\n", from, to);
			return _r::not_found;
		}
		
		printf("from %lu to %lu\n", from, to);
		
		if (unlikely( (to != 0) and (to <= from) ))
			return _r::not_found;
		
		printf("START OF REQUEST HEADERS\n%s\nEND OF REQUEST HEADERS\n", file_id);
		
		this->mysql_query(
			"SELECT m.name, CONCAT(d.name, f.name) "
			"FROM file f "
			"JOIN dir d ON d.id=f.dir "
			"JOIN mimetype m ON m.id=f.mimetype "
			"WHERE f.id=", id
		);
		const char* mimetype = nullptr;
		const char* file_path;
		while(this->mysql_assign_next_row__no_free(&mimetype, &file_path));
		if (mimetype == nullptr){
			this->mysql_free_res();
			return _r::not_found;
		}
		
		printf("file_path = %s\n", file_path);
		
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
		printf("File sz: %lu\n", f_sz);
		
		printf("Seeking\n");
		if (unlikely(fseek(f, from, SEEK_SET))){
			this->mysql_free_res();
			return _r::server_error;
		}
		
		const size_t bytes_to_read = (to) ? (to - from) : block_sz;
		const size_t bytes_read = fread(this->buf + room_for_headers,  1,  bytes_to_read,  f);
		fclose(f);
		
		printf("Read %lu bytes\n", bytes_read);
		
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Cache-Control/1day.c"
			// TODO: Get streaming working. Currently only accepted by browser if entire file is sent.
			//"Transfer-Encoding: chunked\n" // WARNING: This is only valid for HTTP/1.1
			//"Connection: keep-alive\n"
			"Accept-Ranges: bytes\n"
			"Content-Type: ", mimetype, '\n',
			"Content-Range: bytes ", from, '-', from + bytes_read, '/', f_sz, '\n',
			"Content-Length: ", bytes_read, '\n', '\n'
		);
		const size_t headers_len = (uintptr_t)this->itr - (uintptr_t)this->buf;
		printf("Returning headers: %.*s\n",  (int)headers_len,  this->buf);
		memcpy(this->buf + room_for_headers - headers_len,  this->buf,  headers_len);
		
		this->mysql_free_res();
		
		return std::string_view(this->buf + room_for_headers - headers_len,  headers_len + bytes_read);
	}
	
	FunctionSuccessness dl_file(const uint64_t dir_id,  const char* const file_name,  const char* const url,  const bool overwrite_existing){
		FunctionSuccessness rc = FunctionSuccessness::ok;
		static char dst_pth[4096];
		
		const char* dir_name = nullptr;
		this->mysql_query("SELECT name FROM dir WHERE id=", dir_id);
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
	
	std::string_view post__dl(const char* s){
		static char url_buf[4096];
		
		const uint64_t dir_id = a2n<uint64_t>(&s);
		++s;
		
		const char* const tag_ids  = get_comma_separated_ints(&s, '/');
		if (tag_ids == nullptr)
			return _r::not_found;
		const size_t tag_ids_len  = (uintptr_t)s - (uintptr_t)tag_ids;
		++s; // Skip slash
		
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
					
					switch(dl_file(dir_id, file_name, url_buf, is_html_file)){
						case FunctionSuccessness::server_error:
							++n_errors;
						case FunctionSuccessness::ok:
							break;
						case FunctionSuccessness::malicious_request:
							return _r::not_found;
					}
					
					this->mysql_exec(
						"INSERT INTO file"
						"(name, dir)"
						"VALUES(",
							'"', _f::esc, '"', file_name, '"', ',',
							dir_id,
						")"
						"ON DUPLICATE KEY UPDATE dir=dir"
					);
					this->mysql_exec(
						"INSERT INTO file2tag"
						"(file_id, tag_id)"
						"SELECT f.id, t.id "
						"FROM file f "
						"JOIN tag t "
						"WHERE f.name=\"", _f::esc, '"', file_name, "\" "
						  "AND f.dir=", dir_id, " "
						  "AND t.id IN (", _f::strlen, tag_ids, tag_ids_len,") "
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
	
	void tag_parentisation(const char* const child_ids,  const char* const tag_ids,  const size_t child_ids_len,  const size_t tag_ids_len){
		this->mysql_exec(
			"INSERT INTO tag2parent (tag_id, parent_id) "
			"SELECT t.id, p.id " // To ensure client cannot modify tags/files they are not authorised to view
			"FROM tag t "
			"JOIN tag p "
			"WHERE t.id IN (", _f::strlen, child_ids, child_ids_len, ")"
			  "AND p.id IN (", _f::strlen, tag_ids,   tag_ids_len,   ")"
			"ON DUPLICATE KEY UPDATE tag_id=tag_id"
		);
		
		this->mysql_exec(
			"INSERT INTO tag2parent_tree (tag, parent, depth) "
			"SELECT t.id, t2pt.parent, t2pt.depth+1 "
			"FROM tag t " // Seemingly unnecessary JOINs are to ensure client cannot modify tags/files they are not authorised to view
			"JOIN tag p "
			"JOIN tag2parent_tree t2pt ON t2pt.tag=p.id "
			"WHERE t.id IN (", _f::strlen, child_ids, child_ids_len, ")"
			  "AND p.id IN (", _f::strlen, tag_ids,   tag_ids_len,   ")"
			"ON DUPLICATE KEY UPDATE depth=LEAST(tag2parent_tree.depth, t2pt.depth+1)"
		);
		
		this->regenerate_tag_json = true;
		this->regenerate_tag2parent_json = true;
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
		
		this->tag_parentisation(tag_ids, parent_ids, tag_ids_len, parent_ids_len);
		
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
		
		this->tag_parentisation(child_ids, tag_ids, child_ids_len, tag_ids_len);
		
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
		
		this->mysql_exec(
			"INSERT INTO file2tag (tag_id, file_id) "
			"SELECT t.id, f.id " // To ensure client cannot modify tags/files they are not authorised to view
			"FROM tag t "
			"JOIN file f "
			"WHERE t.id IN (", _f::strlen, tag_ids,  tag_ids_len,  ")"
			  "AND f.id IN (", _f::strlen, file_ids, file_ids_len, ")"
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
									#include "html/root.html"
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
							case 't':
								switch(*(s++)){
									case '/':
										switch(*(s++)){
											case ' ':
												// /t/
												return
													#include "headers/return_code/OK.c"
													#include "headers/Content-Type/html.c"
													#include "headers/Cache-Control/1day.c"
													"\n"
													#include "html/tags.html"
												;
										}
										break;
									case ' ':
										// /t#1234
										return
											#include "headers/return_code/OK.c"
											#include "headers/Content-Type/html.c"
											#include "headers/Cache-Control/1day.c"
											"\n"
											#include "html/tag.html"
										;
								}
								break;
							case 'd':
								switch(*(s++)){
									case ' ':
										// /d#1234
										return
											#include "headers/return_code/OK.c"
											#include "headers/Content-Type/html.c"
											#include "headers/Cache-Control/1day.c"
											"\n"
											#include "html/dir.html"
										;
									case '/':
										return
											#include "headers/return_code/OK.c"
											#include "headers/Content-Type/html.c"
											#include "headers/Cache-Control/1day.c"
											"\n"
											#include "html/dirs.html"
										;
									break;
								}
							case 'f':
								switch(*(s++)){
									case ' ':
										// /f#1234
										return
											#include "headers/return_code/OK.c"
											#include "headers/Content-Type/html.c"
											#include "headers/Cache-Control/1day.c"
											"\n"
											#include "html/file.html"
										;
									case '/':
										switch(*(s++)){
											case ' ':
												// /f/
												return std::string_view(
													#include "headers/return_code/OK.c"
													#include "headers/Content-Type/html.c"
													#include "headers/Cache-Control/1day.c"
													"\n"
													#include "html/files.html"
												);
										}
										break;
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
						}
						break;
				}
				break;
		}
		return _r::not_found;
	}
  public:
	RTaggerHandler()
	: regenerate_dir_json(false)
	, regenerate_device_json(false)
	, regenerate_tag_json(false)
	, regenerate_tag2parent_json(false)
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

int s2n(const char* s){
	int n = 0;
	while(*s != 0){
		n *= 10;
		n += *s - '0';
		++s;
	}
	return n;
}

int main(int argc,  char** argv){
	curl_global_init(CURL_GLOBAL_ALL);
	
	char** dummy_argv = argv;
	int port_n = 0;
	while (*(++argv)){
		const char* const arg = *argv;
		if (arg[1] != 0)
			goto help;
		switch(*arg){
			case 'p':
				port_n = s2n(*(++argv));
				break;
			case 'c':
				CACHE_DIR = *(++argv);
				CACHE_DIR_STRLEN = strlen(CACHE_DIR);
				break;
			default:
				goto help;
		}
	}
	
	if (port_n == 0){
		help:
		fprintf(
			stderr,
			"USAGE: ./server p [PORT_NUMBER] c [THUMBNAIL_DIRECTORY]\n"
		);
		return 1;
	}
	
	int dummy_argc = 0;
	folly::Init init(&dummy_argc, &dummy_argv);
	
	if (mysql_library_init(0, NULL, NULL))
		throw compsky::mysql::except::SQLLibraryInit();
	
	compsky::mysql::init_auth(_mysql::buf, _mysql::buf_sz, _mysql::auth, getenv("TAGEM_MYSQL_CFG"));
	compsky::mysql::login_from_auth(_mysql::mysql_obj, _mysql::auth);
	
	uint64_t id;
	const char* name;
	constexpr _r::flag::Dict dict;
	_r::init_json(dict, "SELECT id, name FROM protocol", _r::protocols_json, &id, &name);

	wangle::ServerBootstrap<RTaggerPipeline> server;
	server.childPipeline(std::make_shared<RTaggerPipelineFactory>());
	server.bind(port_n);
	server.waitForStop();
	
	mysql_close(_mysql::mysql_obj);
	mysql_library_end();
	compsky::mysql::wipe_auth(_mysql::buf, _mysql::buf_sz);
	
	curl_global_cleanup();

	return 0;
}
