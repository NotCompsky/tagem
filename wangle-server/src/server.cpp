#include "FrameDecoder.h"
#include "CStringCodec.h"

#include <compsky/mysql/query.hpp>
#include <compsky/asciify/asciify.hpp>

#include <folly/init/Init.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>

#include <mutex>
#include <cstring> // for malloc


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
											default: return not_found;
										}
									default: return not_found;
								}
							default: return not_found;
						}
					default: return not_found;
				}
			default: return not_found;
		}
	}
	
	static char buf[1024 * 1024]; // Needs to be large enough for file thumbnails
	char* itr = nullptr;
	
	static const char* tags_json;
	static const char* protocols_json;
	static const char* devices_json;
	
	
	void asciifiis(char*& itr,  const uint64_t* n,  const char* const* str){
		compsky::asciify::asciify(
			itr,
			'"', *n, '"', ':',
				'"', _f::esc, '"', *str, '"',
			','
		);
	}
	size_t strlens(const uint64_t*,  const char* const* str){
		return std::char_traits<char>::length("\"1234567890123456789\":\"\",") + 2*strlen(*str);
	}
	
	void asciifiis(char*& itr,  const uint64_t* n,  const char* const* str0,  const unsigned* m,  const char* const* str1,  const char* const* str2){
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
	size_t strlens(const uint64_t*,  const char* const* str0,  const unsigned*,  const char* const* str1,  const char* const* str2){
		return std::char_traits<char>::length("\"1234567890123456789\":[\"\",123456789,\"\",\"\"],") + 2*strlen(*str0) + 2*strlen(*str1) + 2*strlen(*str2);
	}
	
	template<typename... Args>
	void init_json(const char* const qry,  const char*& dst,  Args... args){
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
			sz += strlens(args...);;
		}
		sz += 1;
		sz += 1;
		
		char* itr = (char*)malloc(sz);
		dst = const_cast<char*>(itr);
		if(unlikely(itr == nullptr))
			exit(4096);
		
		compsky::asciify::asciify(itr, _headers);
		compsky::asciify::asciify(itr, '{');
		mysql_data_seek(mysql_res, 0); // Reset to first result
		while(compsky::mysql::assign_next_row(mysql_res, &mysql_row, args...)){
			asciifiis(itr, args...);
		}
		if (unlikely(*(itr - 1) == ','))
			// If there was at least one iteration of the loop...
			--itr; // ...wherein a trailing comma was left
		*(itr++) = '}';
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
	
	template<typename... Args>
	void mysql_query(Args... args){
		this->reset_buf_index();
		this->asciify(args...);
		this->mysql_query_using_buf();
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
				"IFNULL(LOWER(HEX(f2h.x)),\"\"),"
				"d.id,"
				"d.name,"
				"f.name,"
				"GROUP_CONCAT(f2t.tag_id),"
				"mt.name,"
				"d.device "
			"FROM file f "
			"JOIN dir d ON d.id=f.dir "
			"JOIN file2tag f2t ON f2t.file_id=f.id "
			"JOIN mimetype mt ON mt.id=f.mimetype "
			"LEFT JOIN file2qt5md5 f2h ON f2h.file=f.id "
			"WHERE f.id=", id, " "
			"GROUP BY f.id"
		);
		
		const char* md5_hash;
		const char* device_id;
		const char* dir_id;
		const char* dir_name;
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
		while(this->mysql_assign_next_row(&md5_hash, &dir_id, &dir_name, &file_name, &tag_ids, &mimetype, &device_id)){
			this->asciify(
				'"', md5_hash, '"', ',',
				dir_id, ',',
				'"', _f::esc, '"',  dir_name, '"', ',',
				'"', _f::esc, '"', file_name, '"', ',',
				'"', tag_ids, '"', ',',
				'"', mimetype, '"', ',',
				 device_id
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
				"IFNULL(LOWER(HEX(f2h.x)), \"\"),"
				//"D.protocol,"
				//"d.id,"
				//"d.name,"
				"f.id,"
				"f.name,"
				"GROUP_CONCAT(f2t.tag_id)"
			"FROM file f "
			//"JOIN dir d ON d.id=f.dir "
			//"JOIN device D ON D.id=d.device "
			"JOIN file2tag f2t ON f2t.file_id=f.id "
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
				"IFNULL(LOWER(HEX(f2h.x)), \"\"),"
				"f.id,"
				"f.name,"
				"GROUP_CONCAT(f2t.tag_id)"
			"FROM file f "
			"JOIN dir d ON d.id=f.dir "
			"JOIN file2tag f2t ON f2t.file_id=f.id "
			"LEFT JOIN file2qt5md5 f2h ON f2h.file=f.id "
			"WHERE d.id=", id, " "
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
	std::string_view return_api(const char* s){
		switch(*(s++)){
			case 'd':
				switch(*(s++)){
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
						return _r::devices_json;
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
						return _r::tags_json;
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
			case 't':
				switch(*(s++)){
					case '.':
						// m.json
						return _r::tags_json;
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
	_r::init_json("SELECT id, name FROM tag", _r::tags_json, &id, &name);
	_r::init_json("SELECT id, name FROM protocol", _r::protocols_json, &id, &name);
	unsigned protocol;
	const char* embed_pre;
	const char* embed_post;
	_r::init_json("SELECT id, name, protocol, embed_pre, embed_post FROM device", _r::devices_json, &id, &name, &protocol, &embed_pre, &embed_post);

	wangle::ServerBootstrap<RTaggerPipeline> server;
	server.childPipeline(std::make_shared<RTaggerPipelineFactory>());
	server.bind(port_n);
	server.waitForStop();
	
	mysql_close(_mysql::mysql_obj);
	mysql_library_end();
	compsky::mysql::wipe_auth(_mysql::buf, _mysql::buf_sz);

	return 0;
}
