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

#ifdef USE_LIBCURL

#include "curl_utils.hpp"
#include "str_utils.hpp"
#include "os.hpp"
#include "errors.hpp"
#include <compsky/dl/curl.hpp>
#include <compsky/utils/ptrdiff.hpp>
#include <cstdio> // for fopen
#include <cstring> // for memccpy


namespace curl {


size_t curl_write_callback(char* contents,  size_t size,  size_t nmemb,  void* buf_itr){
	const size_t n_bytes = size * nmemb;
	memcpy(*reinterpret_cast<char**>(buf_itr),  (char*)contents,  n_bytes);
	*reinterpret_cast<char**>(buf_itr) += n_bytes;
    return n_bytes;
}


size_t dl_buf(const char* const url,  char*& dst_buf_orig){
	char* dst_buf = dst_buf_orig;
	
	compsky::dl::Curl curl(
		CURLOPT_WRITEFUNCTION, curl_write_callback,
		CURLOPT_WRITEDATA, &dst_buf,
		CURLOPT_FOLLOWLOCATION, true
#ifdef DNS_OVER_HTTPS_CLIENT_URL
		, CURLOPT_DOH_URL, DNS_OVER_HTTPS_CLIENT_URL
#endif
	);
	
	return curl.perform(url) ? 0 : (compsky::utils::ptrdiff(dst_buf, dst_buf_orig));
}


void init(){
	curl_global_init(CURL_GLOBAL_ALL);
}


void clean(){
	curl_global_cleanup();
}


} // namespace curl


#endif
