#pragma once

#include <curl/curl.h>


class Curl {
  public:
	CURL* const obj;
	
	template<typename... Args>
	Curl(Args&&... args)
	: obj(curl_easy_init())
	{
		this->init(args...);
	}
	
	void init(){}
	
	template<typename Auto1,  typename Auto2,  typename... Args>
	void init(Auto1 opt,  Auto2 value,  Args&&... args){
		this->set_opt(opt, value);
		this->init(args...);
	}
	
	~Curl(){
		curl_easy_cleanup(this->obj);
	}
	
	template<typename Auto1,  typename Auto2>
	void set_opt(Auto1 opt,  Auto2 value){
		curl_easy_setopt(this->obj, opt, value);
	}
	
	void set_url(const char* const url){
		this->set_opt(CURLOPT_URL, url);
	}
	
	bool perform() const {
		return (curl_easy_perform(this->obj) == CURLE_OK);
	}
	
	bool perform(const char* const url){
		this->set_url(url);
		return this->perform();
	}
	
	bool copy_mimetype(char* buf){
		char* _mimetype = nullptr;
		const auto curl_rc2 = curl_easy_getinfo(this->obj, CURLINFO_CONTENT_TYPE, &_mimetype);
		if (_mimetype and not curl_rc2)
			memccpy(buf, _mimetype, 0, MAX_MIMETYPE_SZ);
		return (_mimetype and not curl_rc2);
	}
	
	bool copy_headers(const char* user_headers,  char* user_agent_buf){
		struct curl_slist* headers = nullptr;
		const char* const user_agent = SKIP_TO_HEADER(12,"User-Agent: ")(user_headers);
		if (user_agent == nullptr)
			return true;
		memccpy(user_agent_buf,  user_agent - 11,  '\r',  1000);
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
		this->set_opt(CURLOPT_HTTPHEADER, headers);
		return false;
	}
};
