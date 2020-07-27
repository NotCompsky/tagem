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
#include "extract-audio.hpp" // for extract_audio
#include "qry.hpp"
#include <compsky/mysql/query.hpp>
#include <stdio.h> // for fprintf
#include <unistd.h> // for execv
#include <sys/wait.h>
#include "../../wangle-server/src/proc.hpp"


namespace _err {
	enum {
		none,
		unspecified,
		bad_arg,
		n
	};
}


// Command Line Options
bool STRIP_TO_FILENAMES = false;
const char* PREFIX = nullptr;
size_t PREFIX_LEN;
const char* EXTRACT_AUDIO_INTO_DIR = nullptr;
bool PRINT_EXTRACTED_AUDI_OUTFILE_PATHS = false;
bool EXTRACT_AUDIO = false;


namespace _mysql {
	MYSQL* obj;
	constexpr static const size_t auth_sz = 512;
	char auth[auth_sz];
}


char OUTPUT_FILENAME[4096];
const char* OUTPUT_ROOT_DIR;
size_t OUTPUT_ROOT_DIR_LEN;
const char* FILE_EXT;
size_t FILE_EXT_LEN;

#ifdef EXEC_CMD
std::vector<char*> COMMAND_ARGS; // Actually const char*, but interfaces with execv, an ancient C function
#endif


template<typename String>
String jump_to_filename(String fp){
	String s = fp;
	while(*fp != 0){
		if (*fp == '/')
			s = fp + 1;
		++fp;
	}
	return s;
}


template<typename String>
String at_last_slash(String s){
	while(*s != 0)
		++s;
	while(*s != '/')
		--s;
	return s;
}


void copy_filename(const char* from,  char* to){
	from = at_last_slash(from);
	to = at_last_slash(to);
	while(*from != 0)
		*(++to) = *(++from);
}


void process_rule(InlistFilterRules& r,  const char* const rule_name,  const char* const out_name){
	static MYSQL_RES* res = nullptr;
	static MYSQL_ROW  row;
	// TODO
	
	compsky::asciify::asciify(OUTPUT_FILENAME + OUTPUT_ROOT_DIR_LEN,  out_name,  FILE_EXT,  '\0');
	
	if (unlikely(r.load(rule_name) == 0)){
		fprintf(stderr,  "No such rule: %s\n",  rule_name);
		if (likely(remove(OUTPUT_FILENAME) == 0))
			fprintf(stderr,  "Deleted");
		else
			fprintf(stderr,  "Failed to delete");
		fprintf(stderr, " file: %s\n",  OUTPUT_FILENAME);
		return;
	}
	r.get_results(res, files_from_bash);
	
	FILE* f = fopen(OUTPUT_FILENAME, "wb");
	if (f == nullptr){
		fprintf(stderr,  "Cannot write to file: %s\n",  OUTPUT_FILENAME);
		return;
	}
	
	static const char* _fp;
	while(compsky::mysql::assign_next_row__no_free(res, &row, &_fp)){
		static char buf[4096];
		memcpy(buf,  _fp,  strlen(_fp) + 1);
		// It is free'd by InlistFilterRules::get_results
		char* fp_ = buf;
		if (STRIP_TO_FILENAMES)
			fp_ = jump_to_filename(fp_);
		if (PREFIX != nullptr){
			memcpy(buf,  PREFIX,  PREFIX_LEN);
			memcpy(buf + PREFIX_LEN,  fp_,  strlen(fp_) + 1);
			fp_ = buf;
		}
		if (EXTRACT_AUDIO_INTO_DIR != nullptr){
			static char buf2[4096];
			memcpy(buf2, EXTRACT_AUDIO_INTO_DIR, strlen(EXTRACT_AUDIO_INTO_DIR));
			memcpy(buf2 + strlen(EXTRACT_AUDIO_INTO_DIR),  jump_to_filename(_fp),  strlen(jump_to_filename(_fp)) + 1); // TODO: Reduce number of memcpy calls (the first one only needed once, not every iteration)
			int rc;
			if (rc = extract_audio(buf2, _fp, EXTRACT_AUDIO, PRINT_EXTRACTED_AUDI_OUTFILE_PATHS)){
				fprintf(stderr,  "Cannot extract audio: error #%d\nFrom %s\nTo   %s\n",  rc,  _fp,  buf2);
				continue;
			}
			copy_filename(buf2, fp_);
			printf("%s\n", fp_);
		}
#ifdef EXEC_CMD
		if (COMMAND_ARGS.size() != 0){
			for (auto i = 0;  i < COMMAND_ARGS.size();  ++i){
				const char* s = COMMAND_ARGS[i];
				if (s[0] == '%'  &&  s[2] == 0){
					switch(s[1]){
						case 'f':
							s = _fp;
							break;
						case 'F':
							s = fp_;
							break;
						default:
							break;
					}
					COMMAND_ARGS[i] = const_cast<char*>(s);
					printf("COMMAND_ARG set to %s\n", s);
				}
			}
			char* const* command_args = COMMAND_ARGS.data();
			auto pid = fork();
			if (pid == 0){
				// Child process
				execv(COMMAND_ARGS[0], command_args);
				// execv should cause the thread to exit. ANY return indicates an error.
				fprintf(stderr,  "Cannot execute command: %s\n",  COMMAND_ARGS[0]);
				exit(_err::unspecified);
			}
			int wstatus;
			const pid_t w = wait(&wstatus);
			if (w == -1){
				fprintf(stderr, "wait() error\n");
			}
		}
#endif
		fprintf(f,  "%s\n",  fp_);
	}
	
	printf("Written to %s\n", OUTPUT_FILENAME);
	
	fclose(f);
}


int main(const int argc,  char** argv){
	/*
	 * USAGE:
	 * tagem-mk-playlist [OPTIONS] [OUTPUT_ROOT_DIRECTORY] [FILE_EXTENSION] [[RULE SETS]]
	 *   where a RuleSet is of the format
	 *     -- EnvironmentalVariable=Value ... 'Rule Name' output_name
	 * OPTIONS
	 *   -b
	 *     Strip to file name
	 *   -c
	 *     Execute a command.
	 *     Arguments *equal* to certain values will be replaced:
	 *       %f -> original file path (from MySQL results)
	 *       %F -> destination file path (what is written into the resulting playlist file)
	 *     However, substring substitution is not performed.
	 *     E.g.
	 *       -c /bin/adb push --sync %f %F '\;'
	 *     WARNING: Bug: %f substitutes only the first assigned result - the same string for each iteration.
	 *   -p
	 *     Prefix
	 *   -x [DIRECTORY]
	 *     *Prepare* to extract audio into the given directory. Only useful in conjunction with -X and/or -v
	 *   -X
	 *     Extract the audio
	 *   -v
	 *     Print the file paths of the extracted audio
	 * E.g.
	 * tagem-mk-playlist /tmp/playlists .m3u -- X=tag1 'Rule Name' output_name -- X=tag1 Y=tag2 'Rule Name' output_name -- X='SELECT name FROM file' 'Rule that passes X into SQL query'
	 */
	
	compsky::mysql::init(_mysql::obj, _mysql::auth, _mysql::auth_sz, getenv("TAGEM_MYSQL_CFG"));
	
	// Parse options
	while(true){
		const char* const opt = *(++argv);
		if (opt[0] != '-'){
			--argv; // For verbosity
			break;
		}
		
		switch(opt[1]){
			case 'b':
				STRIP_TO_FILENAMES = true;
				break;
#ifdef EXEC_CMD
			case 'c':
				while(true){
					const char* const arg = *(++argv);
					if (arg[0] == '\\'  &&  arg[1] == ';'  &&  arg[2] == 0)
						break;
					COMMAND_ARGS.push_back(const_cast<char*>(arg));
				}
				break;
#endif
			case 'p':
				PREFIX = *(++argv);
				PREFIX_LEN = strlen(PREFIX);
				break;
			case 'x':
				EXTRACT_AUDIO_INTO_DIR = *(++argv); // WARNING: Fails totally if does not end with slash. TODO: Fix this.
				break;
			case 'X':
				EXTRACT_AUDIO = true;
				break;
			case 'v':
				PRINT_EXTRACTED_AUDI_OUTFILE_PATHS = true;
				break;
			default:
				return _err::bad_arg;
		}
	}
	
	OUTPUT_ROOT_DIR = *(++argv);
	OUTPUT_ROOT_DIR_LEN = strlen(OUTPUT_ROOT_DIR);
	memcpy(OUTPUT_FILENAME,  OUTPUT_ROOT_DIR,  OUTPUT_ROOT_DIR_LEN);
	if (OUTPUT_ROOT_DIR[OUTPUT_ROOT_DIR_LEN - 1] != '/'){
		OUTPUT_FILENAME[OUTPUT_ROOT_DIR_LEN] = '/';
		++OUTPUT_ROOT_DIR_LEN;
	}
	
	FILE_EXT = *(++argv);
	FILE_EXT_LEN = strlen(FILE_EXT);
	
	char rule_buf[1024]; // Guess at reasonable size
	InlistFilterRules r(_mysql::obj, rule_buf);
	
	/*
	 * Parse RuleSets
	 * 
	 * The "--" allows us to locate the positional arguments, and deduce that all preceding arguments are environmental variables to be set.
	 * This means that the first "--" is unnecessary. It is kept only because it keeps the arguments visually neater in usage.
	 */
	argv += 2; // Skip first "--" (and point to the string afterwards)
	bool not_encountered_end = true;
	while(not_encountered_end){
		int i = 0;
		while(true){
			if (argv[i] == nullptr){
				not_encountered_end = false;
				break;
			}
			if (argv[i][0] == '-'  &&  argv[i][1] == '-'  &&  argv[i][2] == 0)
				break;
			++i;
		}
		
		const int n_env_vars = i - 2;
		for (int j = n_env_vars;  j != 0;  ){
			putenv(argv[--j]);
		}
		
		const char* const rule_name = argv[n_env_vars - 1 + 1];
		const char* const out_name  = argv[n_env_vars - 1 + 2];
		
		process_rule(r, rule_name, out_name);
		
		argv += i + 1;
	}
	
	compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
	
	return _err::none;
}
