#include <compsky/mysql/query.hpp>
#include <boost/regex.hpp>
#include <pHash.h>
#include <audiophash.h>
#include <openssl/sha.h>
extern "C" {
# include <libavformat/avformat.h>
# include <libavcodec/avcodec.h>
}
#include <stdexcept>
#include <stdio.h> // for fprintf


#define MAX_HASH_NAME_LENGTH 10


namespace _mysql {
	MYSQL* obj;
	constexpr static const size_t auth_sz = 512;
	char auth[auth_sz];
	MYSQL_RES* res;
	MYSQL_ROW  row;
}


namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
}


static char* BUF;
constexpr static const size_t BUF_SZ = 1024 * 4096;


static AVFormatContext* av_fmt_ctx;


uint64_t duration_of(const char* fp){
	uint64_t n = 0;
	
	if (avformat_open_input(&av_fmt_ctx, fp, NULL, NULL) != 0){
		fprintf(stderr,  "Unable to open video file: %s\n",  fp);
		return 0;
	}
	
	if (avformat_find_stream_info(av_fmt_ctx, NULL) < 0){
		fprintf(stderr,  "Unknown error processing file: %s\n",  fp);
		goto cleanup;
	}
	
	n = av_fmt_ctx->duration / 1000000;
	
	cleanup:
	avformat_close_input(&av_fmt_ctx);
	
	return n;
}


/* For function overloading */
struct Image{};
struct Video{};
struct Audio{};
struct SHA256_FLAG{};
struct Size{};
struct Duration{};


template<typename FileType,  typename Int>
void asciify_hash(const FileType file_type_flag,  char*& itr,  const Int hash){
	compsky::asciify::asciify(
		itr,
		hash
	);
};


void asciify_hash(const SHA256_FLAG file_type_flag,  char*& itr,  const unsigned char* const hash){
	compsky::asciify::asciify(
		itr,
		'"'
	);
	for (auto i = 0;  i < 32;  ++i){
		const char c = hash[i];
		if (c == 0  ||  c == '"'  ||  c == '\\'){
			compsky::asciify::asciify(
				itr,
				'\\'
			);
		}
		compsky::asciify::asciify(
			itr,
			c ? c : '0'
		);
	}
	compsky::asciify::asciify(
		itr,
		'"'
	);
};


template<typename FileType,  typename Int>
void insert_hashes_into_db(const FileType file_type_flag,  const char* const file_id,  const Int* hashes,  const char* const hash_name,  int length_to_go){
	while(length_to_go != 0) {
		char* itr = BUF;
		
		compsky::asciify::asciify(
			itr,
			"INSERT IGNORE INTO file2", hash_name, " "
			"(file,x)"
			"VALUES"
			"("
		);
		
		do {
			--length_to_go;
			
			compsky::asciify::asciify(
				itr,
				file_id,
				','
			);
			
			asciify_hash(file_type_flag, itr, hashes[length_to_go]);
			
			compsky::asciify::asciify(
				itr,
				"),("
			);
		} while (
			(length_to_go != 0)  &&
			(compsky::asciify::get_index(itr, BUF)  <  BUF_SZ - (1 + 19 + 1 + 19 + 2))
		);
		
		compsky::mysql::exec_buffer(
			_mysql::obj,
			BUF,
			(uintptr_t)itr - (uintptr_t)BUF - std::char_traits<char>::length(",)") // Ignore trailing ",("
		);
	}
};


template<typename FileType,  typename Int>
void insert_hashes_into_db_then_free(const FileType file_type_flag,  const char* const file_id,  const Int* hashes,  const char* const hash_name,  int length_to_go){
	insert_hashes_into_db(file_type_flag, file_id, hashes, hash_name, length_to_go);
	free((void*)hashes); // Just a quirk of the C standard
};


uint64_t get_hash_of_image(const char* const fp){
	uint64_t hash;
	const int rc = ph_dct_imagehash(fp, hash);
	return (rc == 0) ? hash : 0; // rc == 0 is success
}

void save_hash(const Size file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp){
	FILE* f = fopen(fp, "rb");
	if (f == nullptr){
		fprintf(stderr,  "Cannot read file: %s\n",  fp);
		return;
	}
	
	fseek(f, 0, SEEK_END);
	const size_t sz = ftell(f);
	
	insert_hashes_into_db(file_type_flag, file_id, &sz, hash_name, 1);
}

void save_hash(const SHA256_FLAG file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp){
	static unsigned char hash[32 + 1] = {};
	
	static SHA256_CTX sha256;
	SHA256_Init(&sha256);
	
	FILE* f = fopen(fp, "rb");
	if (f == nullptr){
		fprintf(stderr,  "Cannot read file: %s\n",  fp);
		return;
	}
	
	static char buf[4096];
	int i;
	while((i = fread(buf, 1, sizeof(buf), f))  !=  0){
		SHA256_Update(&sha256, buf, i);
	}
	
	SHA256_Final(hash, &sha256);
	
	insert_hashes_into_db(file_type_flag, file_id, &hash, hash_name, 1);
}

void save_hash(const Image file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp){
	const uint64_t hash = get_hash_of_image(fp);
	
	if (unlikely(hash == 0)){
		fprintf(stderr, "Cannot get DCT hash: %s\n", fp);
		return;
	}
	
	insert_hashes_into_db_then_free(file_type_flag, file_id, &hash, hash_name, 1);
}


void save_hash(const Video file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp){
	int length;
	const uint64_t* const hashes = ph_dct_videohash(fp, length);
	
	if (hashes == nullptr  ||  length == 0){
		fprintf(stderr, "Cannot hash video: %s\n", fp);
		return;
	}
	
	insert_hashes_into_db_then_free(file_type_flag, file_id, hashes, hash_name, length);
}


void save_hash(const Audio file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp){
	constexpr static const int sample_rate = 8000;
	constexpr static const int channels = 1;
	int audiobuf_len;
	float* audiobuf = ph_readaudio(fp, sample_rate, channels, nullptr, audiobuf_len);
	
	if (audiobuf == nullptr){
		fprintf(stderr,  "Cannot read audio: %s\n", fp);
		return;
	}
	
	int length;
	const uint32_t* const hashes = ph_audiohash(audiobuf, audiobuf_len, sample_rate, length);
	
	if (hashes == nullptr  ||  length == 0){
		fprintf(stderr, "Cannot hash audio: %s\n", fp);
		return;
	}
	
	insert_hashes_into_db_then_free(file_type_flag, file_id, hashes, hash_name, length);
}


void save_hash(const Duration file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp){
	const uint64_t hash = duration_of(fp);
	insert_hashes_into_db(file_type_flag, file_id, &hash, hash_name, 1);
}


void and_name_regexp(char*& itr,  const char* const file_ext_regexp){
	compsky::asciify::asciify(
		itr,
		"AND name REGEXP '\\.", file_ext_regexp, "'"
	);
#define ADD_NAME_REGEXP_SZ (23+128)
}

void and_name_regexp(char*& itr,  const std::nullptr_t file_ext_regexp){}


struct Options {
	const char* directory;
	bool recursive;
	
	Options()
	: directory(nullptr)
	, recursive(false)
	{}
};


constexpr
bool check_regex(const nullptr_t regex,  const char* const file_name,  const size_t file_name_len){
	return true;
}

bool check_regex(const boost::regex* regex,  const char* const file_name,  const size_t file_name_len){
	static boost::match_results<const char*> what;
	return (boost::regex_search(file_name,  file_name + strlen(file_name),  what,  *regex));
}


template<typename FileType,  typename BoostRegex>
void hash_all_from_dir(const char* const dir_name,  const bool recursive,  const BoostRegex regex,  const FileType file_type_flag,  const char* const hash_name){
	static char file_path[4096];
	static size_t dir_prefix_len = 0;
	
	const size_t dir_len = strlen(dir_name);
	memcpy(file_path + dir_prefix_len,  dir_name,  dir_len);
	dir_prefix_len += dir_len;
	file_path[dir_prefix_len] = '/';
	++dir_prefix_len;
	file_path[dir_prefix_len] = 0;
	
	DIR* dir = opendir(file_path);
	if (dir == nullptr)
		return;
	struct dirent* e;
	while((e = readdir(dir)) != nullptr){
		const char* const ename = e->d_name;
		if (ename == nullptr)
			continue;
		if (
			(ename[0] == '.') &&
			(ename[1] == 0)   || (ename[1] == '.'  &&  ename[2] == 0)
		)
			// Skip . and ..
			continue;
		
		if (e->d_type == DT_DIR){
			if (recursive)
				hash_all_from_dir(ename, recursive, regex, file_type_flag, hash_name);
			continue;
		}
		
		if (e->d_type == DT_LNK)
			continue;
		
		if (!(check_regex(regex,  ename,  strlen(ename)))){
			printf("Failed regex: %s\n", ename);
			continue;
		}
		
		static char buf[128 + 4*4096];
		memcpy(file_path + dir_prefix_len,  ename,  strlen(ename) + 1);
		
		compsky::mysql::exec(
			_mysql::obj,
			buf,
			"INSERT INTO file (name) "
			"SELECT \"", _f::esc, '"', file_path, "\" "
			"FROM file "
			"WHERE NOT EXISTS (SELECT id FROM file WHERE name=\"", _f::esc, '"', file_path, "\") "
			"LIMIT 1"
			// NOTE: ON DUPLICATE KEY skips IDs on duplicate entries due to AUTO_INCREMENT
		);
		compsky::mysql::query(
			_mysql::obj,
			_mysql::res,
			buf,
			"SELECT id FROM file "
			"WHERE name=\"", _f::esc, '"', file_path, "\" "
			  "AND id NOT IN (SELECT file FROM file2", hash_name, ")"
		);
		const char* file_id;
		while(compsky::mysql::assign_next_row(_mysql::res, &_mysql::row, &file_id)){
			// This can only have 0 or 1 iterations
			save_hash(file_type_flag, hash_name, file_id, file_path);
		}
	}
	closedir(dir);
	dir_prefix_len -= (dir_len + 1);
}


template<typename FileType>
void hash_all_from_dir_root(const char* const dirpath,  const bool recursive,  const char* const file_ext_regexp,  const FileType file_type_flag,  const char* const hash_name){
	boost::regex regex(file_ext_regexp, boost::regex::extended); // POSIX extended is the MySQL regex engine
	hash_all_from_dir(dirpath, recursive, &regex, file_type_flag, hash_name);
}


template<typename FileType>
void hash_all_from_dir_root(const char* const dirpath,  const bool recursive,  const nullptr_t file_ext_regexp,  const FileType file_type_flag,  const char* const hash_name){
	hash_all_from_dir(dirpath, recursive, nullptr, file_type_flag, hash_name);
}


template<typename FileType,  typename String>
void hash_all_from(const Options opts,  const FileType file_type_flag,  const String file_ext_regexp,  const char* const hash_name){
	if (opts.directory != nullptr){
		hash_all_from_dir_root(opts.directory, opts.recursive, file_ext_regexp, file_type_flag, hash_name);
		return;
	}
	
	const char* _id;
	const char* _fp;
	
	constexpr static const char* const qry_1 = 
		"SELECT id, name "
		"FROM file "
		"WHERE id NOT IN ("
			"SELECT file "
			"FROM file2" /*, hash_name, */
	;
	
	constexpr static const char* const qry_2 = 
		")"
	;
	
	static char buf[std::char_traits<char>::length(qry_1) + MAX_HASH_NAME_LENGTH + std::char_traits<char>::length(qry_2) + ADD_NAME_REGEXP_SZ]; // NOTE: Requires C++17+ // = qry_1;
	char* itr = buf; // + std::char_traits<char>::length(qry_1);
	
	compsky::asciify::asciify(
		// TODO: Compile-time string concatenation would be nice here, to use query_buffer instead.
		itr,
		qry_1, hash_name, qry_2
	);
	and_name_regexp(itr, file_ext_regexp);
	compsky::mysql::query_buffer(
		_mysql::obj,
		_mysql::res,
		buf,
		(uintptr_t)itr - (uintptr_t)buf
	);
	
	while(compsky::mysql::assign_next_row(_mysql::res, &_mysql::row, &_id, &_fp)){
		try {
			save_hash(file_type_flag, hash_name, _id, _fp);
		} catch (...){
			compsky::mysql::exec(
				// Remove existing hashes so that file appears in the above result next time
				_mysql::obj,
				buf,
				"DELETE FROM file2", hash_name, " WHERE file=", _id
			);
		}
	}
};


int main(const int argc,  const char* const* argv){
	compsky::mysql::init(_mysql::obj, _mysql::auth, _mysql::auth_sz, getenv("TAGEM_MYSQL_CFG"));
	
	BUF = (char*)malloc(BUF_SZ);
	if (BUF == nullptr)
		return 4096;
	
	Options opts;
	do {
		++argv;
		const char* const arg = *argv;
		if (arg == nullptr){
			printf(
				"Usage: %s [OPTIONS] HASH_TYPES\n"
				"	OPTIONS\n"
				"		-d DIRECTORY\n"
				"			Tag all files in a directory (adding them to the database if necessary)\n"
				"		-R\n"
				"	HASH_TYPES a concatenation of any of\n"
				"		a	Audio\n"
				"		d	Duration\n"
				"		i	Image DCT\n"
				"		v	Video DCT\n"
				"		s	SHA256\n"
				"		S	Size\n"
				, *argv
			);
			return 1;
		}
		if (arg[0] != '-')
			break;
		switch(arg[1]){
			case 'd':
				opts.directory = *(++argv);
				break;
			case 'R':
				opts.recursive = true;
				break;
			default:
				// case 0:
				break;
		}
	} while (true);
	
	av_fmt_ctx = avformat_alloc_context();
	
	constexpr static const Image  image_flag;
	constexpr static const Video  video_flag;
	constexpr static const Audio  audio_flag;
	constexpr static const SHA256_FLAG sha256_flag;
	constexpr static const Size   size_flag;
	constexpr static const Duration duration_flag;
	
	const char* const file_types = *argv;
	for (auto i = 0;  i < strlen(file_types);  ++i){
		const char c = file_types[i];
		switch(c){
			case 'a':
				hash_all_from(opts,  audio_flag,  "(mp3|webm|mp4|mkv|avi)$", "audio_hash");
				// WARNING: Ensure ADD_NAME_REGEXP_SZ >= max size of this and similar regexes
				break;
			case 'd':
				hash_all_from(opts,  duration_flag,   "(mp3|webm|mp4|mkv|avi|gif)$", "duration");
				// WARNING: Ensure ADD_NAME_REGEXP_SZ >= max size of this and similar regexes
				break;
			case 'i':
				hash_all_from(opts,  image_flag,  "(png|jpe?g|webp|bmp)$",   "dct_hash");
				break;
			case 'v':
				hash_all_from(opts,  video_flag,  "(gif|webm|mp4|mkv|avi)$", "dct_hash");
				break;
			case 's':
				hash_all_from(opts,  sha256_flag, nullptr, "sha256");
				break;
			case 'S':
				hash_all_from(opts,  size_flag,   nullptr, "size");
				break;
		}
	}
	static_assert(MAX_HASH_NAME_LENGTH == 10);
	
	compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
}
