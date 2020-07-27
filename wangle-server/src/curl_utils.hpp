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

#include "fn_successness.hpp"
#include "read_request.hpp"
#include <cstdio> // for fopen
#include <cstring> // for memccpy
#include <curl/curl.h>


constexpr static const size_t MAX_MIMETYPE_SZ = 100;


FunctionSuccessness dl_file__curl(const char* user_headers,  const char* const url,  const char* const dst_pth,  const bool overwrite_existing,  char* mimetype){
	FunctionSuccessness rc;
	
	if (not overwrite_existing){
		if (fopen(dst_pth, "rb") != nullptr)
			return FunctionSuccessness::ok;
	}
	
	FILE* const f = fopen(dst_pth, "w");
	if (f == nullptr){
		fprintf(stderr,  "Cannot open file for writing: %s\n",  dst_pth);
		return FunctionSuccessness::server_error;
	}
	
	CURL* const handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, f);
	
	/* Copy headers */
	struct curl_slist* headers = nullptr;
	char user_agent_buf[1000];
	const char* const user_agent = SKIP_TO_HEADER(12,"User-Agent: ")(user_headers);
	if (user_agent == nullptr)
		return FunctionSuccessness::malicious_request;
	memccpy(user_agent_buf,  user_agent - 11,  '\r',  sizeof(user_agent_buf));
	replace_first_instance_of(user_agent_buf, '\r', '\0');
	headers = curl_slist_append(headers, user_agent_buf);
	headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
	headers = curl_slist_append(headers, "Accept-Language: en-GB,en;q=0.5");
	headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate, br");
	headers = curl_slist_append(headers, "Connection: keep-alive");
	headers = curl_slist_append(headers, "Upgrade-Insecure-Requests: 1");
	headers = curl_slist_append(headers, "DNT: 1");
	headers = curl_slist_append(headers, "Pragma: no-cache");
	headers = curl_slist_append(headers, "Cache-Control: no-cache");
	headers = curl_slist_append(headers, "TE: Trailers");
	curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
	
	const CURLcode curl_rc = curl_easy_perform(handle);
	fclose(f);
	if (curl_rc != CURLE_OK){
		remove(dst_pth);
		rc = FunctionSuccessness::server_error;
		fprintf(stderr, "dl_file__curl error\n");
	} else {
		char* _mimetype = nullptr;
		const auto curl_rc2 = curl_easy_getinfo(handle, CURLINFO_CONTENT_TYPE, &_mimetype);
		if (_mimetype and not curl_rc2)
			memccpy(mimetype, _mimetype, 0, MAX_MIMETYPE_SZ);
		rc = FunctionSuccessness::ok;
	}
	
	curl_easy_cleanup(handle);
	
	return rc;
}
