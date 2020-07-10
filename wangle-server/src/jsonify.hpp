#pragma once

#include "asciify_flags.hpp"


namespace _r {
		namespace flag {
		struct Dict{};
		struct Arr{};
		
		struct QuoteAndJSONEscape{};
		struct QuoteAndEscape{};
		struct QuoteNoEscape{};
		struct NoQuote{};
	}
	
	constexpr
	void asciify_json_list_response_entry(char*& itr){
		--itr; // Remove trailing comma
	}
	
	template<typename... Args>
	void asciify_json_list_response_entry(char*& itr,  const flag::NoQuote,  const char** str,  Args... args);
	template<typename... Args>
	void asciify_json_list_response_entry(char*& itr,  const flag::QuoteNoEscape,  const char** str,  Args... args);
	template<typename... Args>
	void asciify_json_list_response_entry(char*& itr,  const flag::QuoteAndEscape,  const char** str,  Args... args);
	template<typename... Args>
	void asciify_json_list_response_entry(char*& itr,  const flag::QuoteAndJSONEscape,  const char** str,  Args... args);
	
	template<typename... Args>
	void asciify_json_list_response_entry(char*& itr,  const flag::NoQuote,  const char** str,  Args... args){
		compsky::asciify::asciify(itr, *str, ',');
		asciify_json_list_response_entry(itr, args...);
	}
	template<typename... Args>
	void asciify_json_list_response_entry(char*& itr,  const flag::QuoteNoEscape,  const char** str,  Args... args){
		compsky::asciify::asciify(itr, '"', *str, '"', ',');
		asciify_json_list_response_entry(itr, args...);
	}
	template<typename... Args>
	void asciify_json_list_response_entry(char*& itr,  const flag::QuoteAndEscape,  const char** str,  Args... args){
		compsky::asciify::asciify(itr, '"', _f::esc, '"', *str, '"', ',');
		asciify_json_list_response_entry(itr, args...);
	}
	template<typename... Args>
	void asciify_json_list_response_entry(char*& itr,  const flag::QuoteAndJSONEscape,  const char** str,  Args... args){
		compsky::asciify::asciify(itr, '"', _f::json_esc, *str, '"', ',');
		asciify_json_list_response_entry(itr, args...);
	}
	
	constexpr char opener_symbol(const flag::Dict){ return '{'; }
	constexpr char opener_symbol(const flag::Arr){  return '['; }
	constexpr char closer_symbol(const flag::Dict){ return '}'; }
	constexpr char closer_symbol(const flag::Arr){  return ']'; }
	
	template<typename Flag,  typename... Args>
	void asciify_json_response_entry(char*& itr,  flag::Dict,  const Flag f,  const char** str,  Args... args){
		asciify_json_list_response_entry(itr, f, str);
		compsky::asciify::asciify(itr, ':');
		compsky::asciify::asciify(itr, '[');
		asciify_json_list_response_entry(itr, args...);
		compsky::asciify::asciify(itr, ']');
		compsky::asciify::asciify(itr, ',');
	}
	template<typename... Args>
	void asciify_json_response_entry(char*& itr,  flag::Arr f,  Args... args){
		asciify_json_list_response_entry(itr, args...);
	}
	
	template<typename... Args>
	void asciify_json_response_row(char*& itr,  const flag::Arr f_arr_or_dict,  Args... args){
		compsky::asciify::asciify(itr, '[');
		asciify_json_response_entry(itr, f_arr_or_dict, args...);
		compsky::asciify::asciify(itr, ']');
		compsky::asciify::asciify(itr, ',');
	}
	template<typename Flag>
	void asciify_json_response_row(char*& itr,  const flag::Arr,  const Flag f,  const char** str){
		asciify_json_list_response_entry(itr, f, str);
		compsky::asciify::asciify(itr, ',');
	}
	template<typename Flag,  typename... Args>
	void asciify_json_response_row(char*& itr,  const flag::Dict f_arr_or_dict,  const Flag f,  const char** str,  Args... args){
		asciify_json_list_response_entry(itr, f, str);
		compsky::asciify::asciify(itr, ':');
		constexpr static const flag::Arr _arr;
		asciify_json_response_row(itr, _arr, args...);
	}
	
	
	constexpr
	size_t strlens(){
		return 0;
	}
	template<typename... Args>
	size_t strlens(const flag::NoQuote,  const char** str,  Args... args);
	template<typename... Args>
	size_t strlens(const flag::QuoteNoEscape,  const char** str,  Args... args);
	template<typename... Args>
	size_t strlens(const flag::QuoteAndEscape,  const char** str,  Args... args);
	
	template<typename Int>
	size_t strlens_int(Int m){
		size_t n = 1; // Space for the comma
		do{
			++n;
		}while((m /= 10) != 0);
		return n;
	}
	
	template<typename... Args>
	size_t strlens(const flag::NoQuote,  const char** str,  Args... args){
		return strlen(*str) + 1 /* Trailing comma */ + strlens(args...);
	}
	
	template<typename... Args>
	size_t strlens(const flag::QuoteNoEscape,  const char** str,  Args... args){
		return 1 + strlen(*str) + 1 + 1 + strlens(args...);
	}
	
	template<typename... Args>
	size_t strlens(const flag::QuoteAndEscape,  const char** str,  Args... args){
		return 1 + 2*strlen(*str) + 1 + 1 + strlens(args...);
	}
} // namespace _r
