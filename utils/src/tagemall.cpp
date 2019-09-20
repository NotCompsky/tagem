/*
Usage:
    ./bulk-tag {MYSQL_CONFIG_FILE} {{OPTIONS}} {{RELATIVE_PATHS}} - {{TAGS}}
*/

//#include "../../media/src/file2.hpp"

#include <string.h> // for strlen
#include <unistd.h> // for getcwd
#include <cstdlib> // for malloc and getenv

#include <cstdarg> // To avoid error in MariaDB/mysql.h: ‘va_list’ has not been declared
#include <cstdio> // to avoid printf error
#include <compsky/mysql/query.hpp> // for compsky::mysql::exec


namespace ERR {
    enum {
        NONE,
        UNKNOWN,
		MALLOC,
		BAD_OPTION,
        GETCWD
    };
}


namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
}

int main(const int argc, const char** argv){
	// tagemall [OPTIONS] -- tag1 ... tagN -- filepath1 filepath2 ... filepathM
	constexpr static const size_t buf_sz = 4 * 1024 * 1024;
	char* buf = (char*)malloc(buf_sz);
	if(buf == nullptr)
		return ERR::MALLOC;
	
	std::vector<std::pair<const char*,  const char*>> variable_values;
	
	char cwd[4096]; // For the cncatenation later
	if (getcwd(cwd,  4096 - 1) == NULL)
		return ERR::GETCWD;
	const size_t cwd_len = strlen(cwd);
	cwd[cwd_len] = '/';
	cwd[cwd_len+1] = 0;
    
	MYSQL* mysql_obj;
	constexpr static const size_t mysql_auth_sz = 512;
	char mysql_auth[mysql_auth_sz];
	compsky::mysql::init(mysql_obj, mysql_auth, mysql_auth_sz, getenv("TAGEM_MYSQL_CFG"));
	
	
	unsigned int i = 0;
	while (i < argc){
		const char* arg = argv[++i];
		
		if (arg[0] != '-'  ||  arg[1] == 0  ||  arg[2] != 0)
			break;
		
		switch(arg[0]){
			case '-':
				switch(arg[1]){
					case '-':
						goto break_all__a;
					case 'v':
						// TODO: Find a less confusing short code for this
						variable_values.emplace_back(argv[i+1], argv[i+2]);
						i += 2;
						break;
					default:
						return ERR::BAD_OPTION;
				}
				break;
			default:
				return ERR::BAD_OPTION;
		}
	}
	break_all__a:
	
	
	const unsigned int tag_offset = i + 1;
	unsigned int n_tags = 0;
	while (i < argc){
		const char* arg = argv[++i];
		
		if (arg[0] == '-'  &&  arg[1] == '-'  &&  arg[2] == 0)
			break;
		
		++n_tags;
	}
	
	
	const unsigned int file_offset = i + 1;
	const unsigned int n_files = argc - file_offset;

	
	char* itr;

	
    itr = buf;
	compsky::asciify::asciify(
		itr,
		"INSERT IGNORE INTO tag (name) "
		"VALUES "
	);
	for (auto j = tag_offset;  j < tag_offset + n_tags;  ++j){
		const char* const name = argv[j];
		compsky::asciify::asciify(
			itr,
			'(',
				'"', _f::esc, '"', name, '"',
			')',
			','
		);
	}
	--itr; // Ignore trailing comma
	compsky::mysql::exec_buffer(mysql_obj,  buf,  (uintptr_t)itr - (uintptr_t)buf);
	
	itr = buf;
	compsky::asciify::asciify(
		itr,
		"INSERT IGNORE INTO file (name) "
		"VALUES "
	);
	for (auto j = file_offset;  j < file_offset + n_files;  ++j){
		const char* const name = argv[j];
		compsky::asciify::asciify(
			itr,
			"("
				"\"",
					_f::esc, '"',  (name[0] == '/') ? "" : cwd,
					_f::esc, '"', name,
				"\""
			"),"
		);
	}
	--itr; // Ignore trailing comma
	compsky::mysql::exec_buffer(mysql_obj,  buf,  (uintptr_t)itr - (uintptr_t)buf);
	
	itr = buf;
	compsky::asciify::asciify(
		itr,
		"INSERT IGNORE INTO file2tag (file_id, tag_id) "
		"SELECT f.id, t.id "
		"FROM file f, tag t "
		"WHERE f.name IN ("
	);
	for (auto j = file_offset;  j < file_offset + n_files;  ++j){
		const char* const name = argv[j];
		compsky::asciify::asciify(
			itr,
			'"',
				_f::esc, '"',  (name[0] == '/') ? "" : cwd,
				_f::esc, '"', name,
			'"',
			','
		);
	}
	--itr; // Ignore trailing comma
	compsky::asciify::asciify(
		itr,
		") AND t.name IN ("
	);
	for (auto j = tag_offset;  j < tag_offset + n_tags;  ++j){
		const char* const name = argv[j];
		compsky::asciify::asciify(
			itr,
			'"', _f::esc, '"', name, '"',
			','
		);
	}
	--itr; // Ignore trailing comma
	compsky::asciify::asciify(
		itr,
		")"
	);
	compsky::mysql::exec_buffer(mysql_obj,  buf,  (uintptr_t)itr - (uintptr_t)buf);
	
	for (auto i = 0;  i < variable_values.size();  ++i){
		/*MYSQL_RES* res;
		MYSQL_ROW  row;
		compsky::mysql::query(
			mysql_obj,
			res,
			buf,
			"SELECT conversion "
			"FROM file2 "
			"WHERE name=\"", variable_values[i].first, "\""
		);
		unsigned int _conversion;
		while(compsky::mysql::assign_next_row(res, &row, &_conversion));*/
		
		itr = buf;
		compsky::asciify::asciify(
			itr,
			"INSERT IGNORE INTO file2", variable_values[i].first, " "
			"(file_id, x) "
			"SELECT id, ", variable_values[i].second, " "
			"FROM file "
			"WHERE name IN ("
		);
		for (auto j = file_offset;  j < file_offset + n_files;  ++j){
			const char* const name = argv[j];
			compsky::asciify::asciify(
				itr,
				"\"",
					_f::esc, '"',  (name[0] == '/') ? "" : cwd,
					_f::esc, '"', name,
				"\","
			);
		}
		--itr; // Overwrite trailing comma
		compsky::asciify::asciify(
			itr,
			")"
		);
		compsky::mysql::exec_buffer(mysql_obj,  buf,  (uintptr_t)itr - (uintptr_t)buf);
	}
}
