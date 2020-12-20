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

#include "curl.hpp"
#include "read_request.hpp"
#include "user_agent.hpp" // TODO: Should be set by CMake, to allow different choices, but CMake is just so so so dumb and escaping the characters is such a pain
#include "str_parsing_macros.hpp"
#include <compsky/os/metadata.hpp>
#include <compsky/os/write.hpp>
#include <compsky/asciify/asciify.hpp>
#include <cstddef> // for size_t
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>


namespace curl {


#ifndef USE_LIBCURL


inline
const std::string_view get_url_conn_data(const char* url,  bool& is_https){
	const char* url_itr = url - 1;
	is_https = (IS_STR_EQL(url_itr,5,"https"));
	// url_itr is how either past https (if it is https://...) or past http (if it is http://)
	const char* const host_start = url_itr + 1 + 3;
	return std::string_view(host_start,  count_until(host_start, '/'));
}

inline
const std::string_view get_url_conn_data(std::string_view const url,  bool& is_https){
	return get_url_conn_data(url.data(), is_https);
}

template<typename Url,  typename Mimetype>
size_t dl(Url const url,  char*& dst_buf,  const char* const dst_pth,  Mimetype mimetype);

template<typename Url,  typename Mimetype>
size_t dl(Url const url,  char*& dst_buf,  const char* const dst_pth,  Mimetype mimetype){
	// dst_buf_orig is used to temporarily construct the request string, and stores the response string
	
	constexpr bool is_copying_to_file = std::is_same<char**, Mimetype>::value;
	
	char* dst_buf_orig = dst_buf;
	
	bool is_https;
	const std::string_view host = get_url_conn_data(url, is_https);
	const std::string_view port = is_https ? "443" : "80";
	
	boost::asio::io_service io_service;
	boost::system::error_code err;
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator = boost::asio::ip::tcp::resolver(io_service).resolve(host, port, err);
	
	if (unlikely(err))
		return 0;
	
	char* itr = dst_buf_orig;
	compsky::asciify::asciify(
		itr,
		"GET ", url, " HTTP/1.1\r\n",
		"Host: ", host, "\r\n"
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
	boost::asio::const_buffer request(dst_buf_orig,  (uintptr_t)itr - (uintptr_t)dst_buf_orig);
	
	constexpr size_t max_bytes_to_read = HANDLER_BUF_SZ - 1;
	size_t n_bytes_read;
	
	if (is_https){
		boost::asio::ssl::context ssl_ctx(boost::asio::ssl::context::method::sslv23_client);
		boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket(io_service, ssl_ctx);
		boost::asio::connect(socket.lowest_layer(), endpoint_iterator);
		socket.handshake(boost::asio::ssl::stream_base::handshake_type::client);
		// TODO: Verify certificates
		boost::asio::write(socket, request);
		n_bytes_read = boost::asio::read(
			socket,
			boost::asio::mutable_buffer(dst_buf_orig, max_bytes_to_read),
				// WARNING: Should deduct some amount of bytes, as we have probably filled some content already
			err
		);
	} else {
		boost::asio::ip::tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);
		boost::asio::write(socket, request);
		n_bytes_read = boost::asio::read(
			socket,
			boost::asio::mutable_buffer(dst_buf_orig, max_bytes_to_read),
			err
		);
	}
	if (unlikely(not err)){ // if (unlikely(n_bytes_read == max_bytes_to_read)){}
		// This means it didn't encounter an EOF - i.e. n_bytes_read == max_bytes_to_read
		// boost::asio doesn't actually have a function overload involving a non-dynamically allocated buffer for a read function where you read until EOF *without* an error
		if constexpr (not is_copying_to_file)
			// Do not attempt to enlarge the buffer
			return 0;
	}
	
	const char* response_itr = dst_buf_orig - 1;
	if (unlikely(not IS_STR_EQL(response_itr,9,"HTTP/1.1 ")))
		return 0;
	
	switch(response_itr[1]){
		case '2':
			switch(response_itr[2]){
				case '0':
					switch(response_itr[3]){
						case '0':
							// OK
							goto break_out_of_many_loops;
						case '6':
							// TODO: Deal with partial response
							return 0;
						default:
							return 0;
					}
				default:
					return 0;
			}
		case '3':
			switch(response_itr[2]){
				case '0':
					switch(response_itr[3]){
						case '1':
						case '2':
						case '3':
						case '4':
						case '7':
						case '8': {
							const std::string_view redirect_url = STRING_VIEW_FROM_UP_TO(12, "\r\nLocation: ")(response_itr, '\r');
							return dl(redirect_url, dst_buf, dst_pth, mimetype);
						}
						default:
							return 0;
					}
				default:
					return 0;
			}
		default:
			return 0;
	}
	break_out_of_many_loops:
	
	dst_buf_orig[n_bytes_read] = 0;
	
	char* const start_of_content = const_cast<char*>(SKIP_TO_HTTP_CONTENT(dst_buf_orig));
	if constexpr (not is_copying_to_file)
		dst_buf = start_of_content;
	
	if (unlikely(start_of_content == nullptr))
		// Who on Earth has 11 megabytes' worth of headers???
		return 0;
	
	if constexpr (is_copying_to_file){
		compsky::os::WriteOnlyFile f(dst_pth);
		if (unlikely(f.is_null()))
			return 0;
		do {
			f.write_from_buffer(dst_buf_orig, n_bytes_read);
		} while(n_bytes_read == max_bytes_to_read);
	}
	
	return n_bytes_read;
}


inline
bool dl_file(char* buf,  const char* const url,  const char* const dst_pth,  const bool overwrite_existing,  char*& mimetype){
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
		const size_t sz = os::get_file_sz(dst_pth);
		if ((sz != 0) and (sz != -1))
			return true;
	}
	
	mimetype = nullptr;
	
	FILE* const f = fopen(dst_pth, "wb");
	if (f == nullptr){
		printf("FILE CANOT OPEN\n"); fflush(stdout);
		//log("Cannot open file for writing: ",  dst_pth);
		return false;
	}
	
	Curl curl(
		CURLOPT_WRITEDATA, f,
		CURLOPT_FOLLOWLOCATION, true,
#ifdef DNS_OVER_HTTPS_CLIENT_URL
		CURLOPT_DOH_URL, DNS_OVER_HTTPS_CLIENT_URL
#endif
	);
	
	const bool is_failed = curl.perform(url);
	fclose(f);
	if (unlikely(is_failed)){
		printf("dl_file__curl error\n"); fflush(stdout);
		os::del_file(dst_pth);
		//log("dl_file__curl error");
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
