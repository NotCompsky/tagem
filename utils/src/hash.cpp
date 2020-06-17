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

// Only used for QT5MD5 hash
#include <QUrl>
#include <QByteArray>
#include <QCryptographicHash>
#include <QString>
#include <QFile>


#define MAX_HASH_NAME_LENGTH 10


namespace _mysql {
	MYSQL* obj;
	constexpr static const size_t auth_sz = 512;
	char auth[auth_sz];
}


MYSQL_RES* RES1;
MYSQL_ROW  ROW1;
const char* THUMBNAIL_DIR = nullptr;


namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
}


constexpr static const size_t BUF_SZ = 1024 * 4096;
char BUF[BUF_SZ];


static AVFormatContext* av_fmt_ctx;


static int verbosity = 0;


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
struct MD5_FLAG{};
struct QT5_MD5_FLAG{}; // Used in KDE for thumbnails. A hash of the file url, rather than the contents.
struct Size{};
struct Duration{};


struct ManyToMany {
	constexpr
	static
	const char* const insert_pre_hash_name = "INSERT IGNORE INTO file2";
	constexpr
	static
	const char* const insert_post_hash_name = "(x,file) VALUES (";
	constexpr
	static
	const char* const insert_pre_file_id = ",";
	
	constexpr
	static
	size_t insert_trailing_length = std::char_traits<char>::length(",)");
	
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
	const char* const insert_pre_file_id = " WHERE id=";
	
	constexpr
	static
	size_t insert_trailing_length = std::char_traits<char>::length(",)");
	
	constexpr
	static
	const char* const filter_previously_completed_pre  = "f.";
	constexpr
	static
	const char* const filter_previously_completed_post = " IS NULL";
	constexpr
	static
	const char* const delete_pre = "UPDATE _file SET ";
	constexpr
	static
	const char* const delete_post = "=NULL WHERE id=";
};


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


void asciify_hash(const MD5_FLAG file_type_flag,  char*& itr,  const unsigned char* const hash){
	constexpr static const SHA256_FLAG f;
	asciify_hash(f,  itr,  hash);
};


void asciify_hash(const QT5_MD5_FLAG file_type_flag,  char*& itr,  const unsigned char* const hash){
	constexpr static const SHA256_FLAG f;
	asciify_hash(f,  itr,  hash);
};


template<typename FileType,  typename Int,  typename RelationType>
void insert_hashes_into_db(const FileType file_type_flag,  const char* const file_id,  const Int* hashes,  const char* const hash_name,  int length_to_go,  const RelationType which_relation){
	while(length_to_go != 0) {
		char* itr = BUF;
		
		compsky::asciify::asciify(
			itr,
			which_relation.insert_pre_hash_name, hash_name, which_relation.insert_post_hash_name
		);
		
		do {
			--length_to_go;
			
			asciify_hash(file_type_flag, itr, hashes[length_to_go]);
			
			compsky::asciify::asciify(
				itr,
				which_relation.insert_pre_file_id,
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
template<typename FileType,  typename IntOrIntPtr,  typename RelationType>
void insert_hashes_into_db_then_free(const FileType file_type_flag,  const char* const file_id,  const IntOrIntPtr hashes,  const char* const hash_name,  int length_to_go,  const RelationType which_relation){
	insert_hashes_into_db(file_type_flag, file_id, get_ptr_to_if_not_already(hashes), hash_name, length_to_go, which_relation);
	freemajig(hashes); // Just a quirk of the C standard
};


uint64_t get_hash_of_image(const char* const fp){
	uint64_t hash;
	const int rc = ph_dct_imagehash(fp, hash);
	return (rc == 0) ? hash : 0; // rc == 0 is success
}

template<typename RelationType>
void save_hash(const Size file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp,  const RelationType which_relation){
	FILE* f = fopen(fp, "rb");
	if (f == nullptr){
		fprintf(stderr,  "Cannot read file: %s\n",  fp);
		return;
	}
	
	fseek(f, 0, SEEK_END);
	const size_t sz = ftell(f);
	
	insert_hashes_into_db(file_type_flag, file_id, &sz, hash_name, 1, which_relation);
}

template<typename RelationType>
void save_hash(const SHA256_FLAG file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp,  const RelationType which_relation){
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
	
	insert_hashes_into_db(file_type_flag, file_id, &hash, hash_name, 1, which_relation);
}

template<typename RelationType>
void save_hash(const MD5_FLAG file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp,  const RelationType which_relation){
	static unsigned char hash[MD5_DIGEST_LENGTH + 1] = {};
	
	MD5_CTX md5_ctx;
	MD5_Init(&md5_ctx);
	
	FILE* f = fopen(fp, "rb");
	if (f == nullptr){
		fprintf(stderr,  "Cannot read file: %s\n",  fp);
		return;
	}
	
	static char buf[4096];
	int i;
	while((i = fread(buf, 1, sizeof(buf), f))  !=  0){
		MD5_Update(&md5_ctx, buf, i);
	}
	
	MD5_Final(hash, &md5_ctx);
	
	insert_hashes_into_db(file_type_flag, file_id, &hash, hash_name, 1, which_relation);
}

template<typename RelationType>
void save_hash(const QT5_MD5_FLAG file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp,  const RelationType which_relation){
	static unsigned char hash[16 + 1] = {};
	
	const QUrl url(QString("file://") + QString(fp));
	
	// The following is from https://api.kde.org/frameworks/kio/html/previewjob_8cpp_source.html
	// Slightly modified (some variables designated const)
	// BEGIN copyrighted text
	/*
	Copyright (C) GPLv2 or later
		2000 David Faure <faure@kde.org>
		2000 Carsten Pfeiffer <pfeiffer@kde.org>
		2001 Malte Starostik <malte.starostik@t-online.de>
	*/
	const QByteArray origName = url.toEncoded();
	QCryptographicHash md5(QCryptographicHash::Md5);
	md5.addData(QFile::encodeName(QString::fromUtf8(origName)));
	// END copyrighted text
	
	const QByteArray hash_ba = md5.result();
	const char* hash_signed = hash_ba.data();
	
	memcpy(hash, hash_signed, sizeof(hash));
	
	insert_hashes_into_db(file_type_flag, file_id, &hash, hash_name, 1, which_relation);
	
	
	if (THUMBNAIL_DIR == nullptr)
		return;
	
	
	// The following is from PreviewJobPrivate::statResultThumbnail https://api.kde.org/frameworks/kio/html/previewjob_8cpp_source.html
	// Slightly modified (some variables designated const)
	// BEGIN copyrighted text
	/*
	Copyright (C) GPLv2 or later
		2000 David Faure <faure@kde.org>
		2000 Carsten Pfeiffer <pfeiffer@kde.org>
		2001 Malte Starostik <malte.starostik@t-online.de>
	*/
	const QString thumbName = QString::fromUtf8(QFile::encodeName(QString::fromLatin1(md5.result().toHex()))) + QLatin1String(".png");
	// END copyrighted text
	
	if (QFile::exists(thumbName))
		return;
	
	//QImage img;
	//if (not THUMB_CREATOR.create(url, 256, 256, img)){
		fprintf(stderr, "Failed to create thumbnail for: %s\n", fp);
	//	return;
	//}
	//img.save(thumbName);
}

template<typename RelationType>
void save_hash(const Image file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp,  const RelationType which_relation){
	const uint64_t hash = get_hash_of_image(fp);
	
	if (unlikely(hash == 0)){
		fprintf(stderr, "Cannot get DCT hash: %s\n", fp);
		return;
	}
	
	insert_hashes_into_db_then_free(file_type_flag, file_id, hash, hash_name, 1, which_relation);
}

template<typename RelationType>
void save_hash(const Video file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp,  const RelationType which_relation){
	int length;
	const uint64_t* const hashes = ph_dct_videohash(fp, length);
	
	if (hashes == nullptr  ||  length == 0){
		fprintf(stderr, "Cannot hash video: %s\n", fp);
		return;
	}
	
	insert_hashes_into_db_then_free(file_type_flag, file_id, hashes, hash_name, length, which_relation);
}

template<typename RelationType>
void save_hash(const Audio file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp,  const RelationType which_relation){
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
	
	insert_hashes_into_db_then_free(file_type_flag, file_id, hashes, hash_name, length, which_relation);
}

template<typename RelationType>
void save_hash(const Duration file_type_flag,  const char* const hash_name,  const char* const file_id,  const char* const fp,  const RelationType which_relation){
	const uint64_t hash = duration_of(fp);
	insert_hashes_into_db(file_type_flag, file_id, &hash, hash_name, 1, which_relation);
}


void and_name_regexp(char*& itr,  const char* const file_ext_regexp){
	compsky::asciify::asciify(
		itr,
		" AND f.name REGEXP '\\.", file_ext_regexp, "'"
	);
#define ADD_NAME_REGEXP_SZ (23+128)
}

void and_name_regexp(char*& itr,  const std::nullptr_t file_ext_regexp){}


struct Options {
	char* directory;
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
			printf("Failed regex: %s\n", ename);
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
			save_hash(file_type_flag, hash_name, file_id, file_path, which_relation);
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


template<typename FileType,  typename String,  typename RelationType>
void hash_all_from(const Options opts,  const FileType file_type_flag,  const String file_ext_regexp,  const char* const hash_name,  const RelationType& which_relation){
	if (opts.directory != nullptr){
		hash_all_from_dir_root(opts.directory, opts.recursive, file_ext_regexp, file_type_flag, hash_name, which_relation);
		return;
	}
	
	const char* _id;
	const char* _fp;
	
	static char buf[100 + MAX_HASH_NAME_LENGTH + ADD_NAME_REGEXP_SZ];
	char* itr = buf;
	
	compsky::asciify::asciify(
		// TODO: Compile-time string concatenation would be nice here, to use query_buffer instead.
		itr,
		"SELECT f.id, CONCAT(d.name, f.name) "
		"FROM _file f "
		"JOIN _dir d ON d.id=f.dir "
		"WHERE ",
		which_relation.filter_previously_completed_pre, hash_name, which_relation.filter_previously_completed_post
	);
	and_name_regexp(itr, file_ext_regexp);
	compsky::mysql::query_buffer(
		_mysql::obj,
		RES1,
		buf,
		(uintptr_t)itr - (uintptr_t)buf
	);
	
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &_id, &_fp)){
		try {
			save_hash(file_type_flag, hash_name, _id, _fp, which_relation);
		} catch (...){
			compsky::mysql::exec(
				// Remove existing hashes so that file appears in the above result next time
				_mysql::obj,
				buf,
				which_relation.delete_pre, hash_name, which_relation.delete_post, _id
			);
		}
	}
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
			printf(
				"Usage: [OPTIONS] HASH_TYPES\n"
				"	OPTIONS\n"
				"		-d DIRECTORY\n"
				"			Tag all files in a directory (adding them to the database if necessary)\n"
				"		-R\n"
				"		-v\n"
				"			Add verbosity\n"
				"		-t DIRECTORY\n"
				"			Set thumbnail directory\n"
				"			If Qt5 MD5 is used, will create all non-existing thumbnails\n"
				"	HASH_TYPES a concatenation of any of\n"
				"		a	Audio\n"
				"		d	Duration\n"
				"		i	Image DCT\n"
				"		v	Video DCT\n"
				"		s	SHA256\n"
				"		m	MD5\n"
				"		M	Qt5 MD5 (for thumbnails)\n"
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
			case 'R':
				opts.recursive = true;
				break;
			case 't':
				THUMBNAIL_DIR = *(++argv);
				break;
			case 'v':
				++verbosity;
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
	constexpr static const MD5_FLAG    md5_flag;
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
				hash_all_from(opts,  audio_flag,  "(mp3|webm|mp4|mkv|avi)$", "audio_hash", many_to_many_flag);
				// WARNING: Ensure ADD_NAME_REGEXP_SZ >= max size of this and similar regexes
				break;
			case 'd':
				hash_all_from(opts,  duration_flag,   "(mp3|webm|mp4|mkv|avi|gif)$", "duration", one_to_one_flag);
				// WARNING: Ensure ADD_NAME_REGEXP_SZ >= max size of this and similar regexes
				break;
			case 'i':
				hash_all_from(opts,  image_flag,  "(png|jpe?g|webp|bmp)$",   "dct_hash", many_to_many_flag);
				// many_to_many_flag because they use the same hash as video files.
				break;
			case 'v':
				hash_all_from(opts,  video_flag,  "(gif|webm|mp4|mkv|avi)$", "dct_hash", many_to_many_flag);
				break;
			case 's':
				hash_all_from(opts,  sha256_flag, nullptr, "sha256", one_to_one_flag);
				break;
			case 'm':
				hash_all_from(opts,  md5_flag,    nullptr, "md5", one_to_one_flag);
				break;
			case 'M':
				hash_all_from(opts,  qt5_md5_flag,nullptr, "md5_of_path", one_to_one_flag);
				break;
			case 'S':
				hash_all_from(opts,  size_flag,   nullptr, "size", one_to_one_flag);
				break;
		}
	}
	static_assert(MAX_HASH_NAME_LENGTH == 10);
	
	compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
}
