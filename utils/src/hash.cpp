#define CLI_ONLY
#include "basename.hpp"
#include "protocol.hpp"

#include <compsky/mysql/query.hpp>
#include <boost/regex.hpp>
#include <pHash.h>
#include <audiophash.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
extern "C" {
# include <libavformat/avformat.h>
# include <libavcodec/avcodec.h>
}
#include <stdexcept>
#include <stdio.h> // for fprintf
#include <signal.h>


#define HASH_TYPE_STRUCT(struct_name, hash_name, _is_available_to_backup_files_too) \
	struct struct_name { \
		constexpr static const char* const name = hash_name; \
		constexpr static bool is_available_to_backup_files_too = _is_available_to_backup_files_too; \
	};
#define GET_AV_STREAM_OF_TYPE(type) \
	for (int i = 0;  i < FFMPEG_INPUT_FMT_CTX->nb_streams;  i++){ \
		av_stream = FFMPEG_INPUT_FMT_CTX->streams[i]; \
		/* if output context has audio codec support and current input stream is audio*/ \
		if (av_stream == nullptr  ||  av_stream->codecpar->codec_type != type) \
			continue; \
		break; \
	}

#define MAX_HASH_NAME_LENGTH 10


namespace _mysql {
	MYSQL* obj;
	constexpr static const size_t auth_sz = 512;
	char auth[auth_sz];
}


MYSQL_RES* RES1;
MYSQL_ROW  ROW1;
const char* THUMBNAIL_DIR = nullptr;
AVFormatContext* FFMPEG_INPUT_FMT_CTX;


namespace _f {
	using namespace compsky::asciify::flag;
	constexpr static const Escape esc;
	constexpr static const esc::SpacesAndNonAscii esc_spaces_and_non_ascii;
}


constexpr static const size_t BUF_SZ = 1024 * 4096;
char BUF[BUF_SZ];


static AVFormatContext* av_fmt_ctx;


static int verbosity = 0;

FILE* logfile = stderr;
char* custom_regex = nullptr;


const char* CURRENT_FILE_PATH = nullptr;
const char* CURRENT_HASHING_METHOD = nullptr;

extern "C"
void intercept_exit(int){
	fprintf(logfile,  "Exited.\nLast file being processed: %s\n", CURRENT_FILE_PATH);
	
	if (CURRENT_FILE_PATH != nullptr)
		compsky::mysql::exec(
			_mysql::obj,
			BUF,
			"DELETE f2h "
			"FROM file2", CURRENT_HASHING_METHOD, " f2h "
			"JOIN _file f ON f.id=f2h.file "
			"JOIN _dir d ON d.id=f.dir "
			"WHERE CONCAT(d.name, f.name)=\"", _f::esc, '"', CURRENT_FILE_PATH, "\""
		);
	
	compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
	
	exit(0);
}

extern "C"
void intercept_abort(int _signal){
	fprintf(logfile,  "Signal: %s.\nLast processed:\n\tHash Kind: %s\n\tFile Path: %s\n", (_signal==SIGKILL)?"SIGKILL":"SIGABRT", CURRENT_HASHING_METHOD, CURRENT_FILE_PATH);
	
	if (CURRENT_FILE_PATH != nullptr)
		compsky::mysql::exec(
			_mysql::obj,
			BUF,
			"INSERT INTO hash_abortions__", CURRENT_HASHING_METHOD, " "
			"(file)"
			"SELECT f.id "
			"FROM _file f "
			"JOIN _dir d ON d.id=f.dir "
			"WHERE CONCAT(d.name, f.name)=\"", _f::esc, '"', CURRENT_FILE_PATH, "\""
		);
	
	compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
	
	exit(1);
}


uint64_t duration_of(const char* fp){
	uint64_t n = 0;
	
	if (avformat_open_input(&av_fmt_ctx, fp, NULL, NULL) != 0){
		fprintf(logfile,  "Unable to open video file: %s\n",  fp);
		return 0;
	}
	
	if (avformat_find_stream_info(av_fmt_ctx, NULL) < 0){
		fprintf(logfile,  "Unknown error processing file: %s\n",  fp);
		goto cleanup;
	}
	
	n = av_fmt_ctx->duration / 1000000;
	
	cleanup:
	avformat_close_input(&av_fmt_ctx);
	
	return n;
}


/* For function overloading */
HASH_TYPE_STRUCT(Image, "image", false)
HASH_TYPE_STRUCT(Video, "video", false)
HASH_TYPE_STRUCT(Audio, "audio", false)
HASH_TYPE_STRUCT(SHA256_FLAG, "sha256", false)
HASH_TYPE_STRUCT(MD5_FLAG, "md5", false)
HASH_TYPE_STRUCT(MimeType, "mimetype", true)
HASH_TYPE_STRUCT(QT5_MD5_FLAG, "qt5md5", false) // Used in KDE for thumbnails. A hash of the file url, rather than the contents.
HASH_TYPE_STRUCT(Size, "size", false)
HASH_TYPE_STRUCT(Duration, "duration", false)


struct ManyToMany {
	constexpr
	static
	const char* const insert_pre_hash_name = "INSERT IGNORE INTO file2";
	constexpr
	static
	const char* const insert_post_hash_name = "(x,file) VALUES (";
	constexpr
	static
	const char* const backup_insert_pre_hash_name = "INSERT IGNORE INTO file_backup2";
	constexpr
	static
	const char* const insert_pre_file_id = ",";
	
	constexpr
	static
	size_t insert_trailing_length = std::char_traits<char>::length(",(");
	
	constexpr
	static
	const char* const filter_previously_completed_pre  = "f.id NOT IN (SELECT file FROM file2";
	constexpr
	static
	const char* const filter_previously_completed_post = ")";
	constexpr
	static
	const char* const delete_pre = "DELETE FROM file2";
	constexpr
	static
	const char* const delete_post = " WHERE file=";
};
struct OneToOne {
	constexpr
	static
	const char* const insert_pre_hash_name = "UPDATE _file SET ";
	constexpr
	static
	const char* const insert_post_hash_name = "=";
	constexpr
	static
	const char* const backup_insert_pre_hash_name = "UPDATE file_backup SET ";
	constexpr
	static
	const char* const insert_pre_file_id = " WHERE id=";
	constexpr
	static
	const char* const backup_insert_pre_file_id = " WHERE file=";
	
	constexpr
	static
	size_t insert_trailing_length = std::char_traits<char>::length("),(");
	
	constexpr
	static
	const char* const filter_previously_completed_pre  = "IFNULL(f.";
	constexpr
	static
	const char* const filter_previously_completed_post = ",0)=0";
	constexpr
	static
	const char* const delete_pre = "UPDATE _file SET ";
	constexpr
	static
	const char* const delete_post = "=NULL WHERE id=";
};


template<size_t sz,  typename FileType,  typename Int>
// sz of 0 indicates integer type, or non-constlength string; sz above 0 indicates const-length string
void asciify_hash(const FileType file_type_flag,  char*& itr,  const Int hash){
	static_assert(sz == 0);
	compsky::asciify::asciify(
		itr,
		hash
	);
};


template<size_t sz>
void asciify_hash(const SHA256_FLAG file_type_flag,  char*& itr,  const unsigned char* const hash){
	static_assert(sz != 0);
	compsky::asciify::asciify(
		itr,
		'"'
	);
	for (auto i = 0;  i < sz;  ++i){
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

template<size_t sz>
void asciify_hash(const MimeType,  char*& itr,  const unsigned char* const hash){
	static_assert(sz == 0);
	compsky::asciify::asciify(itr, '"', hash, '"');
};


template<size_t sz>
void asciify_hash(const MD5_FLAG file_type_flag,  char*& itr,  const unsigned char* const hash){
	constexpr static const SHA256_FLAG f;
	asciify_hash<sz>(f,  itr,  hash);
};

template<size_t sz>
void asciify_hash(const QT5_MD5_FLAG file_type_flag,  char*& itr,  const unsigned char* const hash){
	constexpr static const SHA256_FLAG f;
	asciify_hash<sz>(f,  itr,  hash);
};


template<size_t hash_sz,  typename FileType,  typename Int,  typename RelationType>
void insert_hashes_into_db(const FileType file_type_flag,  const char* const file_id,  const uint64_t dir_id,  const Int* hashes,  const char* const hash_name,  int length_to_go,  const RelationType which_relation){
	while(length_to_go != 0) {
		char* itr = BUF;
		
		if (dir_id == 0){
			compsky::asciify::asciify(
				itr,
				which_relation.insert_pre_hash_name, hash_name, which_relation.insert_post_hash_name
			);
		} else {
			compsky::asciify::asciify(
				itr,
				which_relation.backup_insert_pre_hash_name, hash_name, which_relation.insert_post_hash_name
			);
		}
		
		do {
			--length_to_go;
			
			asciify_hash<hash_sz>(file_type_flag, itr, hashes[length_to_go]);
			
			compsky::asciify::asciify(
				itr,
				which_relation.insert_pre_file_id,
				" AND ", (dir_id==0)?"0=":"dir=", dir_id,
				file_id
			);
			
			compsky::asciify::asciify(
				itr,
				"),("
			);
		} while (
			(length_to_go != 0)  &&
			((uintptr_t)itr - (uintptr_t)BUF)  <  BUF_SZ - (1 + 19 + 1 + 19 + 2)
		);
		
		compsky::mysql::exec_buffer(
			_mysql::obj,
			BUF,
			(uintptr_t)itr - (uintptr_t)BUF - which_relation.insert_trailing_length // Ignore trailing ",(", or "),(" if OneToOne relation
		);
	}
};


template<typename Int>
void freemajig(Int hash){}
template<typename Int>
void freemajig(Int* hashes){
	free((void*)hashes);
}
template<typename Int>
Int* get_ptr_to_if_not_already(Int& hash){
	return &hash;
}
template<typename Int>
Int* get_ptr_to_if_not_already(Int* hashes){
	return hashes;
}
template<typename Int>
bool is_nullptr(Int const ptr){
	return false;
}
template<typename Int>
bool is_nullptr(Int* const ptr){
	return ptr == nullptr;
}
template<size_t hash_sz,  typename FileType,  typename IntOrIntPtr,  typename RelationType>
void insert_hashes_into_db_then_free(const char* const fp,  const FileType file_type_flag,  const char* const file_id,  const uint64_t dir_id,  const IntOrIntPtr hashes,  const char* const hash_name,  int length_to_go,  const RelationType which_relation){
	if (is_nullptr(hashes))
		goto no_hashes_found__230jf0jfe;
	
	if (length_to_go == 0){
		free((void*)hashes);
		no_hashes_found__230jf0jfe:
		fprintf(logfile, "Cannot hash %s: %s\n", file_type_flag.name, fp);
		return;
	}
	
	insert_hashes_into_db<hash_sz>(file_type_flag, file_id, dir_id, get_ptr_to_if_not_already(hashes), hash_name, length_to_go, which_relation);
	freemajig(hashes); // Just a quirk of the C standard
};


uint64_t get_hash_of_image(const char* const fp){
	uint64_t hash;
	const int rc = ph_dct_imagehash(fp, hash);
	return (rc == 0) ? hash : 0; // rc == 0 is success
}

template<typename RelationType>
void save_hash(const Size file_type_flag,  const char* const hash_name,  const char* const file_id,  const uint64_t dir_id,  const char* const fp,  const RelationType which_relation){
	FILE* f = fopen(fp, "rb");
	if (f == nullptr){
		fprintf(logfile,  "Cannot read file: %s\n",  fp);
		return;
	}
	
	fseek(f, 0, SEEK_END);
	const size_t sz = ftell(f);
	
	fclose(f);
	
	insert_hashes_into_db<0>(file_type_flag, file_id, dir_id, &sz, hash_name, 1, which_relation);
}

template<typename RelationType>
void save_hash(const SHA256_FLAG file_type_flag,  const char* const hash_name,  const char* const file_id,  const uint64_t dir_id,  const char* const fp,  const RelationType which_relation){
	static unsigned char hash[32] = {};
	
	static SHA256_CTX sha256;
	SHA256_Init(&sha256);
	
	FILE* f = fopen(fp, "rb");
	if (f == nullptr){
		fprintf(logfile,  "Cannot read file: %s\n",  fp);
		return;
	}
	
	static char buf[4096];
	int i;
	while((i = fread(buf, 1, sizeof(buf), f))  !=  0){
		SHA256_Update(&sha256, buf, i);
	}
	
	fclose(f);
	
	SHA256_Final(hash, &sha256);
	
	insert_hashes_into_db<32>(file_type_flag, file_id, dir_id, &hash, hash_name, 1, which_relation);
}

template<typename RelationType>
void save_hash(const MimeType file_type_flag,  const char* const hash_name,  const char* const file_id,  const uint64_t dir_id,  const char* const fp,  const RelationType which_relation){
	const char* const* hashes;
	const AVCodecDescriptor* av_codec_descr;
	AVStream* av_stream;
	
	if (avformat_open_input(&FFMPEG_INPUT_FMT_CTX, fp, nullptr, nullptr)){
		fprintf(logfile,  "FFMPEG cannt open file: %s\n",  fp);
		goto cleanup1;
	}
	if (avformat_find_stream_info(FFMPEG_INPUT_FMT_CTX, nullptr)){
		fprintf(logfile,  "FFMPEG cannt find stream info in file: %s\n",  fp);
		goto cleanup2;
	}
	
	GET_AV_STREAM_OF_TYPE(AVMEDIA_TYPE_VIDEO)
	if (av_stream == nullptr)
		GET_AV_STREAM_OF_TYPE(AVMEDIA_TYPE_AUDIO)
	if (av_stream == nullptr){
		fprintf(logfile,  "Cannot find audio or video stream: %s\n",  fp);
		goto cleanup2;
	}
	
	av_codec_descr = avcodec_descriptor_get(av_stream->codecpar->codec_id);
	if(av_codec_descr==nullptr){
		fprintf(logfile,  "File has no video codec descriptor: %s\n",  fp);
		goto cleanup2;
	}
	hashes = av_codec_descr->mime_types;
	if(hashes==nullptr){
		fprintf(logfile,  "File's codec has no associated mimetype: %s\n",  fp);
		goto cleanup2;
	}
	fprintf(logfile,  "    File has mimetype: %s: %s\n",  hashes[0],  fp);
	insert_hashes_into_db<0>(file_type_flag, file_id, dir_id, hashes, hash_name, 1, which_relation);
	
	cleanup2:
	avformat_close_input(&FFMPEG_INPUT_FMT_CTX);
	
	cleanup1:
	avformat_free_context(FFMPEG_INPUT_FMT_CTX);
}

template<typename RelationType>
void save_hash(const MD5_FLAG file_type_flag,  const char* const hash_name,  const char* const file_id,  const uint64_t dir_id,  const char* const fp,  const RelationType which_relation){
	static unsigned char hash[MD5_DIGEST_LENGTH] = {};
	
	MD5_CTX md5_ctx;
	MD5_Init(&md5_ctx);
	
	FILE* f = fopen(fp, "rb");
	if (f == nullptr){
		fprintf(logfile,  "Cannot read file: %s\n",  fp);
		return;
	}
	
	static char buf[4096];
	int i;
	while((i = fread(buf, 1, sizeof(buf), f))  !=  0){
		MD5_Update(&md5_ctx, buf, i);
	}
	
	fclose(f);
	
	MD5_Final(hash, &md5_ctx);
	
	insert_hashes_into_db<MD5_DIGEST_LENGTH>(file_type_flag, file_id, dir_id, &hash, hash_name, 1, which_relation);
}

template<typename RelationType>
void save_hash(const QT5_MD5_FLAG file_type_flag,  const char* const hash_name,  const char* const file_id,  const uint64_t dir_id,  const char* const fp,  const RelationType which_relation){
	static unsigned char hash[16] = {};
	
	MD5_CTX md5_ctx;
	MD5_Init(&md5_ctx);
	static char buf[7 + 4096];
	compsky::asciify::asciify(buf, "file://", _f::esc_spaces_and_non_ascii, fp, '\0');
	MD5_Update(&md5_ctx, buf, strlen(buf));
	MD5_Final(hash, &md5_ctx);
	
	insert_hashes_into_db<16>(file_type_flag, file_id, dir_id, &hash, hash_name, 1, which_relation);
}

template<typename RelationType>
void save_hash(const Image file_type_flag,  const char* const hash_name,  const char* const file_id,  const uint64_t dir_id,  const char* const fp,  const RelationType which_relation){
	const uint64_t hash = get_hash_of_image(fp);
	
	if (unlikely(hash == 0)){
		fprintf(logfile, "Cannot get DCT hash: %s\n", fp);
		return;
	}
	
	insert_hashes_into_db_then_free<0>(fp, file_type_flag, file_id, dir_id, hash, hash_name, 1, which_relation);
}


template<typename RelationType>
void save_hash(const Video file_type_flag,  const char* const hash_name,  const char* const file_id,  const uint64_t dir_id,  const char* const fp,  const RelationType which_relation){
	// Set global variables for catching aborts
	CURRENT_FILE_PATH = fp;
	CURRENT_HASHING_METHOD = "dct_hash";
	
	int length;
	const uint64_t* const hashes = ph_dct_videohash(fp, length);
	
	insert_hashes_into_db_then_free<0>(fp, file_type_flag, file_id, dir_id, hashes, hash_name, length, which_relation);
}

template<typename RelationType>
void save_hash(const Audio file_type_flag,  const char* const hash_name,  const char* const file_id,  const uint64_t dir_id,  const char* const fp,  const RelationType which_relation){
	constexpr static const int sample_rate = 8000;
	constexpr static const int channels = 1;
	int audiobuf_len;
	float* audiobuf = ph_readaudio(fp, sample_rate, channels, nullptr, audiobuf_len);
	
	if (audiobuf == nullptr){
		fprintf(logfile,  "Cannot read audio: %s\n", fp);
		return;
	}
	
	int length;
	const uint32_t* const hashes = ph_audiohash(audiobuf, audiobuf_len, sample_rate, length);
	
	insert_hashes_into_db_then_free<0>(fp, file_type_flag, file_id, dir_id, hashes, hash_name, length, which_relation);
}

template<typename RelationType>
void save_hash(const Duration file_type_flag,  const char* const hash_name,  const char* const file_id,  const uint64_t dir_id,  const char* const fp,  const RelationType which_relation){
	const uint64_t hash = duration_of(fp);
	insert_hashes_into_db<0>(file_type_flag, file_id, dir_id, &hash, hash_name, 1, which_relation);
}


void and_dir_regexp(char*& itr,  const char* const dir_regexp){
	compsky::asciify::asciify(
		itr,
		" AND d.name REGEXP \"", _f::esc, '"', dir_regexp, "\""
	);
#define ADD_DIR_REGEXP_SZ (23+128)
}

void and_dir_regexp(char*&,  const std::nullptr_t){}


void and_name_regexp(char*& itr,  const char* const file_ext_regexp){
	compsky::asciify::asciify(
		itr,
		" AND f.name REGEXP '\\.", file_ext_regexp, "'"
	);
#define ADD_NAME_REGEXP_SZ (23+128)
}

void and_name_regexp(char*&,  const std::nullptr_t){}


struct Options {
	char* directory;
	char* device_regexp;
	bool recursive;
	
	Options()
	: directory(nullptr)
	, device_regexp(".")
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


template<typename FileType,  typename BoostRegex,  typename RelationType>
void hash_all_from_dir(const char* const dir_name,  const bool recursive,  const BoostRegex regex,  const FileType file_type_flag,  const char* const hash_name,  const RelationType& which_relation){
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
				hash_all_from_dir(ename, recursive, regex, file_type_flag, hash_name, which_relation);
			continue;
		}
		
		if (e->d_type == DT_LNK)
			continue;
		
		if (!(check_regex(regex,  ename,  strlen(ename)))){
			fprintf(logfile, "Failed regex: %s\n", ename);
			continue;
		}
		
		static char buf[128 + 4*4096];
		
		file_path[dir_prefix_len] = 0;
		insert_file_from_path_pair(file_path, ename, protocol::local_filesystem);
		
		compsky::mysql::query(
			_mysql::obj,
			RES1,
			buf,
			"SELECT f.id "
			"FROM file f "
			"JOIN dir d ON d.id=f.dir "
			"WHERE d.name=\"", _f::esc, '"', file_path, "\" "
			  "AND f.name=\"", _f::esc, '"', ename, "\" "
			  "AND ", which_relation.filter_previously_completed_pre, hash_name, which_relation.filter_previously_completed_post
		);
		memcpy(file_path + dir_prefix_len,  ename,  strlen(ename) + 1);
		const char* file_id;
		while(compsky::mysql::assign_next_row(RES1, &ROW1, &file_id)){
			// This can only have 0 or 1 iterations
			save_hash(file_type_flag, hash_name, file_id, 0, file_path, which_relation);
		}
	}
	closedir(dir);
	dir_prefix_len -= (dir_len + 1);
}


template<typename FileType,  typename RelationType>
void hash_all_from_dir_root(const char* const dirpath,  const bool recursive,  const char* const file_ext_regexp,  const FileType file_type_flag,  const char* const hash_name,  const RelationType& which_relation){
	boost::regex regex(file_ext_regexp, boost::regex::extended); // POSIX extended is the MySQL regex engine
	hash_all_from_dir(dirpath, recursive, &regex, file_type_flag, hash_name, which_relation);
}


template<typename FileType,  typename RelationType>
void hash_all_from_dir_root(const char* const dirpath,  const bool recursive,  const nullptr_t file_ext_regexp,  const FileType file_type_flag,  const char* const hash_name,  const RelationType& which_relation){
	hash_all_from_dir(dirpath, recursive, nullptr, file_type_flag, hash_name, which_relation);
}


template<typename FileType,  typename String1,  typename String2,  typename RelationType>
void hash_all_from(const Options opts,  const FileType file_type_flag,  const String1 dir_regexp,  const String2 file_ext_regexp,  const char* const hash_name,  const RelationType& which_relation){
	if (opts.directory != nullptr){
		hash_all_from_dir_root(opts.directory, opts.recursive, file_ext_regexp, file_type_flag, hash_name, which_relation);
		return;
	}
	
	const char* _id;
	const char* _fp;
	uint64_t _dir_id;
	
	static char buf[100 + MAX_HASH_NAME_LENGTH + ADD_DIR_REGEXP_SZ + ADD_NAME_REGEXP_SZ];
	char* itr = buf;
	
	unsigned is_available_to_backup_files_too = file_type_flag.is_available_to_backup_files_too;
	do {
	compsky::asciify::asciify(
		// TODO: Compile-time string concatenation would be nice here, to use query_buffer instead.
		itr,
		"SELECT "
			"f.id,",
			(is_available_to_backup_files_too)?"d.id":"0", ","
			"CONCAT(d.name, f.name) "
		"FROM _file f "
		"JOIN _dir d ON d.id=f.dir "
		"JOIN _device D ON D.id=d.device "
		"WHERE f.id NOT IN (SELECT file FROM hash_abortions__", hash_name, ") "
		  "AND D.name REGEXP \"", opts.device_regexp, "\" "
		  "AND ",
		which_relation.filter_previously_completed_pre, hash_name, which_relation.filter_previously_completed_post
	);
	and_dir_regexp(itr,  dir_regexp);
	and_name_regexp(itr, file_ext_regexp);
	compsky::mysql::query_buffer(
		_mysql::obj,
		RES1,
		buf,
		(uintptr_t)itr - (uintptr_t)buf
	);
	
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &_id, &_dir_id, &_fp)){
		try {
			save_hash(file_type_flag, hash_name, _id, _dir_id, _fp, which_relation);
		} catch (...){
			compsky::mysql::exec(
				// Remove existing hashes so that file appears in the above result next time
				_mysql::obj,
				buf,
				which_relation.delete_pre, hash_name, which_relation.delete_post, _id
			);
		}
	}
	} while(--is_available_to_backup_files_too);
};


void ensure_endswith_slash(char* str){
	while(*str != 0)
		++str;
	--str;
	if (*str == '/')
		*str = 0;
}


int main(const int argc,  char* const* argv){
	compsky::mysql::init(_mysql::obj, _mysql::auth, _mysql::auth_sz, getenv("TAGEM_MYSQL_CFG"));
	
	Options opts;
	do {
		++argv;
		const char* const arg = *argv;
		if (arg == nullptr){
			fprintf(
				stderr,
				"Usage: [OPTIONS] HASH_TYPES\n"
				"	OPTIONS\n"
				"		-d DIRECTORY\n"
				"			Tag all files in a directory (adding them to the database if necessary)\n"
				"		-R\n"
				"			Recursive\n"
				"		-v\n"
				"			Add verbosity\n"
				"		-t DIRECTORY\n"
				"			Set thumbnail directory\n"
				"			If Qt5 MD5 is used, will create all non-existing thumbnails\n"
				"		-D [REGEXP]\n"
				"			Limit tagging to a device\n"
				"			Double quotes must be escaped\n"
				"		-L /path/to/log/file.txt\n"
				"		-r [REGEXP]\n"
				"			Hash files whose names (not full paths) match the regexp\n"
				"	HASH_TYPES a concatenation of any of\n"
				"		a	Audio\n"
				"		d	Duration\n"
				"		i	Image DCT\n"
				"		v	Video DCT\n"
				"		s	SHA256\n"
				"		m	MD5\n"
				"		M	Mime Type\n"
				"		p	Qt5 MD5 (for thumbnails)\n"
				"		S	Size\n"
			);
			return 1;
		}
		if (arg[0] != '-')
			break;
		switch(arg[1]){
			case 'd':
				opts.directory = *(++argv);
				ensure_endswith_slash(opts.directory);
				break;
			case 'D':
				opts.device_regexp = *(++argv);
				break;
			case 'R':
				opts.recursive = true;
				break;
			case 't':
				THUMBNAIL_DIR = *(++argv);
				break;
			case 'v':
				++verbosity;
			case 'L':
				logfile = fopen(*(++argv), "wb");
				assert(logfile != nullptr);
				break;
			case 'r':
				custom_regex = *(++argv);
				break;
			default:
				// case 0:
				break;
		}
	} while (true);
	
	signal(SIGINT,  &intercept_exit);
	signal(SIGKILL, &intercept_abort);
	signal(SIGABRT, &intercept_abort);
	
	av_fmt_ctx = avformat_alloc_context();
	
	constexpr static const Image  image_flag;
	constexpr static const Video  video_flag;
	constexpr static const Audio  audio_flag;
	constexpr static const SHA256_FLAG sha256_flag;
	constexpr static const MD5_FLAG    md5_flag;
	constexpr static const MimeType    mimetype_flag;
	constexpr static const QT5_MD5_FLAG qt5_md5_flag;
	constexpr static const Size   size_flag;
	constexpr static const Duration duration_flag;
	
	constexpr static const ManyToMany many_to_many_flag;
	constexpr static const OneToOne one_to_one_flag;
	
	const char* const file_types = *argv;
	for (auto i = 0;  i < strlen(file_types);  ++i){
		const char c = file_types[i];
		switch(c){
			case 'a':
				hash_all_from(opts,  audio_flag,    "^/",  (custom_regex!=nullptr)?custom_regex:"(mp3|webm|mp4|mkv|avi)$", "audio_hash", many_to_many_flag);
				// WARNING: Ensure ADD_NAME_REGEXP_SZ >= max size of this and similar regexes
				break;
			case 'd':
				hash_all_from(opts, duration_flag,  "^/",  (custom_regex!=nullptr)?custom_regex:"(mp3|webm|mp4|mkv|avi|gif)$", "duration", one_to_one_flag);
				// WARNING: Ensure ADD_NAME_REGEXP_SZ >= max size of this and similar regexes
				break;
			case 'i':
				hash_all_from(opts,  image_flag,    "^/",  (custom_regex!=nullptr)?custom_regex:"(png|jpe?g|webp|bmp)$",   "dct_hash", many_to_many_flag);
				// many_to_many_flag because they use the same hash as video files.
				break;
			case 'v':
				hash_all_from(opts,  video_flag,    "^/",  (custom_regex!=nullptr)?custom_regex:"(gif|webm|mp4|mkv|avi)$", "dct_hash", many_to_many_flag);
				break;
			case 's':
				if(custom_regex!=nullptr)
					abort();
				hash_all_from(opts,  sha256_flag,   "^/",  nullptr, "sha256", one_to_one_flag);
				break;
			case 'm':
				if(custom_regex!=nullptr)
					abort();
				hash_all_from(opts,  md5_flag,      "^/",  nullptr, "md5", one_to_one_flag);
				break;
			case 'M':
				FFMPEG_INPUT_FMT_CTX = avformat_alloc_context();
				if(unlikely(FFMPEG_INPUT_FMT_CTX==nullptr))
					abort();
				hash_all_from(opts,  mimetype_flag,   "^/",  (custom_regex!=nullptr)?custom_regex:"(gif|webm|mp4|mkv|avi)$", "mimetype", one_to_one_flag);
				break;
			case 'p':
				if(custom_regex!=nullptr)
					abort();
				hash_all_from(opts,  qt5_md5_flag,  "^/",     nullptr, "md5_of_path", one_to_one_flag);
				break;
			case 'S':
				if(custom_regex!=nullptr)
					abort();
				hash_all_from(opts,  size_flag,     "^/",  nullptr, "size", one_to_one_flag);
				break;
		}
	}
	static_assert(MAX_HASH_NAME_LENGTH == 10);
	
	compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
}
