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
#pragma once

#include "user_agent.hpp" // TODO: Should be set by CMake, to allow different choices, but CMake is just so so so dumb and escaping the characters is such a pain
#include <compsky/str/parse.hpp>
#include "log.hpp"
#include <compsky/os/metadata.hpp>
#include <compsky/os/write.hpp>
#include <compsky/os/del.hpp>
#include <compsky/asciify/asciify.hpp>
#include <compsky/utils/ptrdiff.hpp>
#ifdef USE_LIBCURL
# include <compsky/dl/curl.hpp>
#else
# include <compsky/dl/asio.hpp>
#endif
#include <cstddef> // for size_t


namespace curl {


#ifndef USE_LIBCURL


namespace _detail {

inline
const std::string_view get_host(std::string_view url){
	// NOTE: Assuming that url begins with http
	url.remove_suffix( (url.at(4) == 's') ? 5 : 4 );
	const size_t new_len = url.find_first_of('/');
	return std::string_view(url.data(), new_len);
}

inline
const std::string_view get_host(const char* url){
	// NOTE: Assuming that url begins with http
	url += (url[4] == 's') ? 5 : 4;
	const size_t new_len = compsky::str::count_until(url, '/');
	return std::string_view(url, new_len);
}

} // namespace _detail


template<typename Url,  typename Mimetype>
size_t dl(Url const url,  char*& dst_buf,  const char* const dst_pth,  Mimetype mimetype){
	char* itr = dst_buf;
	compsky::asciify::asciify(
		itr,
		"GET ", url, " HTTP/1.1\r\n",
		"Host: ", _detail::get_host(url), "\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"User-Agent: " USER_AGENT "\r\n"
		"Accept-Language: en-GB,en;q=0.5\r\n"
		// TODO: Accept gzip, which makes it less obviously automated. "Accept-Encoding: gzip, deflate, br\r\n"
		"Upgrade-Insecure-Requests: 1\r\n"
		"DNT: 1\r\n"
		"Pragma: no-cache\r\n"
		"Cache-Control: no-cache\r\n"
		"TE: Trailers\r\n"
		"\r\n"
	);
	compsky::dl::asio::dl(url,  std::string_view(dst_buf,  compsky::utils::ptrdiff(itr, dst_buf)),  dst_buf,  dst_pth,  mimetype);
}


template<typename Url>
bool dl_file(char* buf,  const Url url,  const char* const dst_pth,  const bool overwrite_existing,  char*& mimetype){
	// nonzero success
	
	if (not overwrite_existing)
		if (compsky::os::get_file_sz(dst_pth) != 0)
			return true;
	
	return dl(url, buf, dst_pth, &mimetype);
}


inline
size_t dl_buf(const char* const url,  char*& dst_buf){
	return dl(url, dst_buf, nullptr, nullptr);
}


inline
void init(){}


inline
void clean(){}


#else


template<typename Str>
bool dl_file(char*,  const Str url,  const char* const dst_pth,  const bool overwrite_existing,  char*& mimetype){
	// nonzero success
	// NOTE: Overwrites dst_pth if it is empty.
	
	if (not overwrite_existing){
		const size_t sz = compsky::os::get_file_sz(dst_pth);
		if ((sz != 0) and (sz != -1))
			return true;
	}
	
	mimetype = nullptr;
	
	FILE* const f = fopen(dst_pth, "wb");
	if (f == nullptr){
		log("Cannot open file for writing: ",  dst_pth);
		return false;
	}
	
	compsky::dl::Curl curl(
		CURLOPT_WRITEDATA, f,
		CURLOPT_FOLLOWLOCATION, true
#ifdef DNS_OVER_HTTPS_CLIENT_URL
		, CURLOPT_DOH_URL, DNS_OVER_HTTPS_CLIENT_URL
#endif
	);
	
	const bool is_failed = curl.perform(url);
	fclose(f);
	if (unlikely(is_failed)){
		compsky::os::del_file(dst_pth);
		log("dl_file__curl error");
		return false;
	} else {
		return true;
	}
}

size_t dl_buf(const char* const url,  char*& dst_buf);

void init();

void clean();


#endif // ifndef USE_LIBCURL


} // namespace curl
