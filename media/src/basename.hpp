#pragma once

#include <compsky/mysql/query.hpp>


namespace _mysql {
	extern MYSQL* obj;
}
extern char BUF[];


namespace protocol {
	enum {
		NONE,
		local_filesystem,
		http,
		https,
		COUNT
	};
}


inline
const char* basename(const char* s){
	const char* end_of_dirname = s;
	while(*s != 0){
		if (*s == '/')
			end_of_dirname = s;
		++s;
	}
	return end_of_dirname + 1;
}


inline
size_t pardir_length(const char* s){
	return reinterpret_cast<uintptr_t>(basename(s)) - reinterpret_cast<uintptr_t>(s);
}


inline
size_t pardir_length(const QString& s){
	int end_of_dirname__indx = 0;
	for(auto i = 0;  i < s.size();  ){
		if (s.at(i) == QChar('/'))
			end_of_dirname__indx = i;
		++i;
	}
	return end_of_dirname__indx + 1;
}


inline
unsigned guess_protocol(const char* url){
	switch(*url){
		case 'h':
			switch(*(++url)){
				case 't':
					switch(*(++url)){
						case 't':
							switch(*(++url)){
								case 'p':
									switch(*(++url)){
										case 's':
											return protocol::https;
										case '/':
											return protocol::http;
									}
							}
					}
			}
		case '/':
			return protocol::local_filesystem;
	}
	return protocol::NONE;
}


inline
void record_dir_from_filepath(const char* const _file_path){
	constexpr static const compsky::asciify::flag::Escape f_esc;
	constexpr static const compsky::asciify::flag::StrLen f_strlen;
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"INSERT INTO dir (name) "
		"SELECT \"", f_esc, '"', f_strlen, pardir_length(_file_path), _file_path, "\" "
		"FROM dir "
		"WHERE NOT EXISTS ("
			"SELECT id "
			"FROM dir "
			"WHERE name=\"", f_esc, '"', f_strlen, pardir_length(_file_path), _file_path, "\""
		")"
		"LIMIT 1"
	);
	// WARNING: What happens if user tries to insert a directory which already exists in '_dir', but which they do not have permission to view (thus not appearing in the 'dir' view)?
}


inline
void update_file_from_path(const uint64_t _file_id,  const char* const _file_path){
	constexpr static const compsky::asciify::flag::Escape f_esc;
	constexpr static const compsky::asciify::flag::StrLen f_strlen;
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"UPDATE file "
		"SET "
			"name=\"", f_esc, '"', basename(_file_path), "\","
			"dir=(SELECT id FROM dir WHERE name=\"", f_esc, '"', f_strlen, pardir_length(_file_path), _file_path, "\")"
		"WHERE id=", _file_id
	);
}


inline
void insert_file_from_path(const char* const _file_path){
	constexpr static const compsky::asciify::flag::Escape f_esc;
	constexpr static const compsky::asciify::flag::StrLen f_strlen;
	compsky::mysql::exec(
		_mysql::obj,
		BUF,
		"INSERT INTO file (dir, name) VALUES ("
			"(SELECT id FROM dir WHERE name=\"", f_esc, '"', f_strlen, pardir_length(_file_path), _file_path, "\"),"
			"\"", f_esc, '"', basename(_file_path), "\""
		") ON DUPLICATE KEY UPDATE dir=dir"
	);
}
