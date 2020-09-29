#pragma once

#include <compsky/macros/likely.hpp>
#include <cstddef> // for size_t
#ifdef _WIN32
#else
# include <sys/stat.h>
#endif


namespace os {


constexpr char path_sep =
  #ifdef _WIN32
	'\\'
  #else
	'/'
  #endif
;

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


} // namespace os
