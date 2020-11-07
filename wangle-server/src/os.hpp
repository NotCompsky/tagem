#pragma once

#include <compsky/macros/likely.hpp>
#include <cstddef> // for size_t
#ifdef _WIN32
#else
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <dirent.h>
# include <fcntl.h> // for O_*
# include <stdio.h> // for remove
#endif


namespace os {


#ifdef _WIN32
typedef ??? dir_handler_typ;
constexpr char path_sep = '\\';
#else
typedef DIR* dir_handler_typ;
typedef struct dirent* dirent_typ;
constexpr char path_sep = '/';
#endif

constexpr char unix_path_sep = '/';
// NOTE: Windows now supports UNIX path separators, and this greatly simplifies things. The database internally only uses UNIX path separators, and client-side Javascript for Windows servers actually converts the path separators before requests are sent to the server.

#ifdef _WIN32
# define PATH_OF_DIRECTORY_EXAMPLE "C:\\Path\\Of\\Directory\\"
#else
# define PATH_OF_DIRECTORY_EXAMPLE "/path/of/directory"
#endif

#ifdef _WIN32
# define JS__REPLACE_PATH_SEP ".replace('\\','/')"
#else
# define JS__REPLACE_PATH_SEP
#endif

constexpr
bool is_local_file_or_dir(const char* path){
  #ifdef _WIN32
	return ((path[0] >= 'A') && (path[0] <= 'Z')) and (path[1] == ':') and (path[2] == path_sep);
  #else
	return (path[0] == path_sep);
  #endif
}

inline
void get_file_size_and_ctime(const char* const fp,  size_t& sz,  time_t& t){
	// NOTE: On a failure, t is set to 0 - not sz - to allow for empty files.
  #ifdef _WIN32
	HANDLE const f = CreateFileA(fp,  GENERIC_READ,  0,  nullptr,  OPEN_EXISTING,  FILE_ATTRIBUTE_NORMAL,  nullptr);
	if (unlikely(f == INVALID_HANDLE_VALUE))
		handler(CANNOT_CREATE_FILE);
	_LARGE_INTEGER f_sz; // For x86_32 compatibility
	LPFILETIME ctime, atime, wtime;
	if (unlikely(GetFileSizeEx(f, &f_sz) == 0))
		sz = 0;
	if (unlikely(GetFileTime(f, ctime, atime, wtime) == 0))
		t = 0;
	CloseHandle(f);
	sz = f_sz.QuadPart;
	t  = *ctime;
  #else
	struct stat stat_buf;
	if (unlikely(stat(fp, &stat_buf) == -1)){
		t = 0;
		return;
	}
	sz = stat_buf.st_size;
	t  = stat_buf.st_ctime;
  #endif
}

inline
size_t get_file_sz(const char* const fp){
  #ifdef _WIN32
	HANDLE const f = CreateFileA(fp,  GENERIC_READ,  0,  nullptr,  OPEN_EXISTING,  FILE_ATTRIBUTE_NORMAL,  nullptr);
	if (unlikely(f == INVALID_HANDLE_VALUE))
		handler(CANNOT_CREATE_FILE);
	_LARGE_INTEGER f_sz; // For x86_32 compatibility
	if (unlikely(GetFileSizeEx(f, &f_sz) == 0))
		handler(COULD_NOT_GET_FILE_SIZE);
	CloseHandle(f);
	return f_sz.QuadPart;
  #else
	struct stat stat_buf;
	if (unlikely(stat(fp, &stat_buf) == -1))
		return 0;
	return stat_buf.st_size;
  #endif
}

inline
bool file_exists(const char* const fp){
  #ifdef _WIN32
	HANDLE const f = CreateFileA(fp,  GENERIC_READ,  0,  nullptr,  OPEN_EXISTING,  FILE_ATTRIBUTE_NORMAL,  nullptr);
	return (f != INVALID_HANDLE_VALUE);
  #else
	static struct stat stat_buf;
	return (stat(fp, &stat_buf) == 0);
  #endif
}

inline
dir_handler_typ open_dir(const char* const path){
#ifdef _WIN32
#else
	return opendir(path);
#endif
}

inline
bool get_next_item_in_dir(dir_handler_typ dir,  dirent_typ& item){
#ifdef _WIN32
#else
	item = readdir(dir);
	return (likely(item != nullptr));
#endif
}

inline
bool is_dir(const dirent_typ item){
#ifdef _WIN32
#else
	return (item->d_type == DT_DIR);
#endif
}

inline
const char* get_dirent_name(const dirent_typ item){
#ifdef _WIN32
#else
	return item->d_name;
#endif
}

inline
bool is_not_file_or_dir_of_interest(const char* const name){
	return (
		(name == nullptr) ||
		(
			(name[0] == '.') &&
			(
				(name[1] == 0) ||
				((name[1] == '.') && (name[2] == 0))
			)
		)
	);
}

inline
void close_dir(dir_handler_typ dir){
#ifdef _WIN32
#else
	closedir(dir);
#endif
}

inline
bool dir_exists(const char* const path){
#ifdef _WIN32
#else
	DIR* const dir = opendir(path);
	const bool b = (dir != nullptr);
	closedir(dir);
	return b;
#endif
}

inline
bool write_to_file(const char* const fp,  const char* const data,  const size_t n_bytes){
#ifdef _WIN32
#else
	const int fd = open(fp,  O_WRONLY | O_CREAT,  S_IRUSR | S_IWUSR | S_IXUSR);
	const bool success = (likely(write(fd, data, n_bytes) == n_bytes));
	close(fd);
	return not success;
#endif
}

inline
size_t read_from_file(const char* const fp,  char* buf,  const size_t n_bytes){
#ifdef _WIN32
#else
	const int fd = open(fp,  O_RDONLY);
	const size_t sz = read(fd, buf, n_bytes);
	close(fd);
	return sz;
#endif
}

inline
size_t read_from_file_at_offset(const char* const fp,  char* buf,  const size_t offset,  const size_t n_bytes){
#ifdef _WIN32
#else
	const int fd = open(fp,  O_RDONLY);
	lseek(fd, offset, SEEK_SET); // TODO: Test for error
	const size_t sz = read(fd, buf, n_bytes);
	close(fd);
	return sz;
#endif
}

inline
bool del_file(const char* const path){
#ifdef _WIN32
#else
	return remove(path);
#endif
}


} // namespace os
