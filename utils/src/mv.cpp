#include <compsky/mysql/query.hpp>
#include <cstdio> // for fprintf, rename
#include <unistd.h> // for getcwd
#include <sys/types.h>
#include <sys/stat.h>


namespace _err {
	enum {
		none,
		unspecified,
		bad_arg,
		syntax,
		cannot_rename,
		getcwd,
		n
	};
}


namespace _mysql {
	MYSQL* obj;
	constexpr static const size_t auth_sz = 512;
	char auth[auth_sz];
}


int main(const int argc,  char** argv){
	constexpr static const compsky::asciify::flag::Escape f_esc;
	
	char cwd[4096]; // For the cncatenation later
	if (getcwd(cwd,  4096 - 1) == NULL)
		return _err::getcwd;
	const size_t cwd_len = strlen(cwd);
	cwd[cwd_len] = '/';
	cwd[cwd_len+1] = 0;
	
	compsky::mysql::init(_mysql::obj, _mysql::auth, _mysql::auth_sz, getenv("TAGEM_MYSQL_CFG"));
	char buf[4 * 4096  +  128];
	
	if (argc == 1)
		return _err::unspecified;
	
	const char* const orig = argv[1];
	const char* const dest = argv[2];
	
	const bool is_dir = ( (orig[strlen(orig) - 1] == '/')  ||  (dest[strlen(dest) - 1] == '/') );
	
	if (!is_dir){
	} else if (orig[strlen(orig) - 1] != '/'){
		fprintf(stderr, "orig path does not end in /\n");
		return _err::unspecified;
	} else if (dest[strlen(dest) - 1] != '/'){
		fprintf(stderr, "dest path does not end in /\n");
		return _err::unspecified;
	}
	
	struct stat info;
	if (stat(dest, &info) == 0){
		const char* ftype;
		if (info.st_mode & S_IFDIR)
			ftype = "directory";
		else
			ftype = "file";
		fprintf(stderr, "%s exists: %s\n", ftype, dest);
		return _err::unspecified;
	}
	
	const int rc = rename(orig, dest);
	
	if (rc != 0){
		fprintf(stderr, "rename() failed\n");
		return _err::unspecified;
	}
	
	if (is_dir){
		char name_qts_escaped[2 * 4096];
		char* itr = name_qts_escaped;
		
		compsky::asciify::asciify(
			itr,
			f_esc, '"', orig,
			'\0'
		);
		
		compsky::mysql::exec(
			_mysql::obj,
			buf,
			"UPDATE file "
			"SET name=CONCAT(\"",
				f_esc, '"', dest,
			"\", SUBSTRING(name, 1+LENGTH(\"", name_qts_escaped, "\"))) "
			"WHERE name LIKE \"", f_esc, '%', name_qts_escaped, "%\""
		);
	} else {
		compsky::mysql::exec(
			_mysql::obj,
			buf,
			"UPDATE file "
			"SET name=\"", f_esc, '"', dest, "\" "
			"WHERE name=\"", f_esc, '"', orig, "\""
		);
	}
	
	compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
	
	return _err::none;
}
