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
		NO_DBL_DASH_AFTER_OPTIONS,
        GETCWD
    };
}


namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
}

struct Variable {
	const char* const name;
	const char* const value;
	const char type;
	Variable(const char* const _name,  const char* const _value,  const char _type)
	: name(_name)
	, value(_value)
	, type(_type)
	{}
};

int main(const int argc, const char** argv){
	/*
	tagemall [OPTIONS] -- tag1 ... tagN -- filepath1 filepath2 ... filepathM
	
	OPTIONS
		v VAR_TYPE VAR_NAME VAR_VALUE
			This is the file2VAR_NAME
			Valid VAR_TYPES are:
				i (integer)
				s (string)
	
	EXAMPLE
		tagemall -v size 4098 -v duration 109 -- -- /media/sars2/co/vid.mp4
			Assigns no tags, only registers the file and assigns file2size and file2duration values
	*/
	
	constexpr static const size_t buf_sz = 4 * 1024 * 1024;
	char* buf = (char*)malloc(buf_sz);
	if(buf == nullptr)
		return ERR::MALLOC;
	
	std::vector<Variable> variable_values;
	
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
			return ERR::NO_DBL_DASH_AFTER_OPTIONS;
		
		switch(arg[0]){
			case '-':
				switch(arg[1]){
					case '-':
						goto break_all__a;
					case 'v':
						// TODO: Find a less confusing short code for this
						variable_values.emplace_back(argv[i+2], argv[i+3], argv[i+1][0]);
						i += 3;
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
		"INSERT INTO _file (name) "
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
	compsky::asciify::asciify(
		itr,
		" ON DUPLICATE KEY UPDATE name=name" // i.e. IGNORE
	);
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
	
	for (const Variable var : variable_values){
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
			"INSERT IGNORE INTO file2", var.name, " "
			"(file, x) "
			"SELECT id, "
		);
		switch(var.type){
			case 'i':
				compsky::asciify::asciify(itr, var.value);
				break;
			case 's':
				char _insert_string_var[1024];
				compsky::mysql::exec(
					mysql_obj,
					_insert_string_var,
					"INSERT IGNORE INTO file2_", var.name, " "
					"(s)"
					"VALUES"
					"("
						"\"", _f::esc, '"', var.value, "\""
					")"
				);
				compsky::asciify::asciify(
					itr,
					"("
						"SELECT x "
						"FROM file2_", var.name, " "
						"WHERE s=\"", _f::esc, '"', var.value, "\""
					")"
				);
				break;
		}
		compsky::asciify::asciify(itr,
			" "
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
