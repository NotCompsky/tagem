#include <compsky/mysql/query.hpp>
#include <pHash.h>
#include <audiophash.h>
#include <openssl/sha.h>
#include <stdio.h> // for fprintf


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


char* BUF;
constexpr static const size_t BUF_SZ = 1024 * 4096;


/* For function overloading */
struct Image{};
struct Video{};
struct Audio{};
struct SHA256_FLAG{};


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
			"INSERT IGNORE INTO file2", hash_name, "_hash "
			"(file,hash)"
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
	return (rc == 0) ? 0 : hash;
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
	
	if (hash == 0)
		return;
	
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


template<typename FileType>
void hash_all_from_db(const FileType file_type_flag,  const char* const file_ext_regexp,  const char* const hash_name){
	const char* _id;
	const char* _fp;
	
	compsky::mysql::query(
		// TODO: Compile-time string concatenation would be nice here, to use query_buffer instead.
		_mysql::obj,
		_mysql::res,
		BUF,
		"SELECT id, name "
		"FROM file "
		"WHERE id NOT IN ("
			"SELECT file "
			"FROM file2", hash_name, "_hash"
		") AND name REGEXP '\\.(", file_ext_regexp, ")$'"
	);
	while(compsky::mysql::assign_next_row(_mysql::res, &_mysql::row, &_id, &_fp)){
		try {
			save_hash(file_type_flag, hash_name, _id, _fp);
		} catch (...){
			fprintf(stderr,  "Killed\n");
			compsky::mysql::exec(
				// Remove existing hashes so that file appears in the above result next time
				_mysql::obj,
				BUF,
				"DELETE FROM file2", hash_name, "_hash WHERE file=", _id
			);
		}
	}
};


int main(const int argc,  const char** const argv){
	compsky::mysql::init(_mysql::obj, _mysql::auth, _mysql::auth_sz, getenv("TAGEM_MYSQL_CFG"));
	
	BUF = (char*)malloc(BUF_SZ);
	if (BUF == nullptr)
		return 4096;
	
	constexpr static const Image  image_flag;
	constexpr static const Video  video_flag;
	constexpr static const Audio  audio_flag;
	constexpr static const SHA256_FLAG sha256_flag;
	
	const char* const file_types = argv[1];
	for (auto i = 0;  i < strlen(file_types);  ++i){
		const char c = file_types[i];
		switch(c){
			case 'a':
				hash_all_from_db(audio_flag,  "mp3|webm|mp4|mkv|avi", "audio");
				break;
			case 'i':
				hash_all_from_db(image_flag,  "png|jpe?g|webp|bmp",   "dct");
				break;
			case 'v':
				hash_all_from_db(video_flag,  "gif|webm|mp4|mkv|avi", "dct");
				break;
			case 's':
				hash_all_from_db(sha256_flag, ".*", "sha256");
				break;
		}
	}
	
	compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
}
