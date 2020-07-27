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
	
	template<size_t col_indx>
	void asciify_json_list_response_entry(char*& itr,  MYSQL_ROW row){
		--itr; // Remove trailing comma
	}
	
	template<size_t col_indx,  typename... Args>
	void asciify_json_list_response_entry(char*& itr,  MYSQL_ROW row,  const flag::NoQuote,  Args... args);
	template<size_t col_indx,  typename... Args>
	void asciify_json_list_response_entry(char*& itr,  MYSQL_ROW row,  const flag::QuoteNoEscape,  Args... args);
	template<size_t col_indx,  typename... Args>
	void asciify_json_list_response_entry(char*& itr,  MYSQL_ROW row,  const flag::QuoteAndEscape,  Args... args);
	template<size_t col_indx,  typename... Args>
	void asciify_json_list_response_entry(char*& itr,  MYSQL_ROW row,  const flag::QuoteAndJSONEscape,  Args... args);
	
	template<size_t col_indx,  typename... Args>
	void asciify_json_list_response_entry(char*& itr,  MYSQL_ROW row,  const flag::NoQuote,  Args... args){
		compsky::asciify::asciify(itr, row[col_indx], ',');
		asciify_json_list_response_entry<col_indx+1>(itr, row, args...);
	}
	template<size_t col_indx,  typename... Args>
	void asciify_json_list_response_entry(char*& itr,  MYSQL_ROW row,  const flag::QuoteNoEscape,  Args... args){
		compsky::asciify::asciify(itr, '"', row[col_indx], '"', ',');
		asciify_json_list_response_entry<col_indx+1>(itr, row, args...);
	}
	template<size_t col_indx,  typename... Args>
	void asciify_json_list_response_entry(char*& itr,  MYSQL_ROW row,  const flag::QuoteAndEscape,  Args... args){
		compsky::asciify::asciify(itr, '"', _f::esc, '"', row[col_indx], '"', ',');
		asciify_json_list_response_entry<col_indx+1>(itr, row, args...);
	}
	template<size_t col_indx,  typename... Args>
	void asciify_json_list_response_entry(char*& itr,  MYSQL_ROW row,  const flag::QuoteAndJSONEscape,  Args... args){
		compsky::asciify::asciify(itr, '"', _f::json_esc, row[col_indx], '"', ',');
		asciify_json_list_response_entry<col_indx+1>(itr, row, args...);
	}
	
	constexpr char opener_symbol(const flag::Dict){ return '{'; }
	constexpr char opener_symbol(const flag::Arr){  return '['; }
	constexpr char closer_symbol(const flag::Dict){ return '}'; }
	constexpr char closer_symbol(const flag::Arr){  return ']'; }
	
	template<typename Flag,  typename... Args>
	void asciify_json_response_entry(char*& itr,  MYSQL_ROW row,  flag::Dict,  const Flag f,  Args... args){
		asciify_json_list_response_entry<0>(itr, row, f);
		compsky::asciify::asciify(itr, ':');
		compsky::asciify::asciify(itr, '[');
		asciify_json_list_response_entry<1>(itr, row, args...);
		compsky::asciify::asciify(itr, ']');
		compsky::asciify::asciify(itr, ',');
	}
	template<typename... Args>
	void asciify_json_response_entry(char*& itr,  MYSQL_ROW row,  flag::Arr f,  Args... args){
		asciify_json_list_response_entry<0>(itr, row, args...);
	}
	
	template<typename... Args>
	void asciify_json_response_row(char*& itr,  MYSQL_ROW row,  const flag::Arr f_arr_or_dict,  Args... args){
		compsky::asciify::asciify(itr, '[');
		asciify_json_response_entry(itr, row, f_arr_or_dict, args...);
		compsky::asciify::asciify(itr, ']');
		compsky::asciify::asciify(itr, ',');
	}
	template<typename ArrOrDict,  typename Flag>
	void asciify_json_response_row(char*& itr,  MYSQL_ROW row,  const ArrOrDict,  const Flag f){
		asciify_json_list_response_entry<0>(itr, row, f);
		compsky::asciify::asciify(itr, ',');
	}
	template<typename Flag1,  typename Flag2>
	void asciify_json_response_row(char*& itr,  MYSQL_ROW row,  const flag::Dict,  const Flag1 f,  const Flag2 g){
		asciify_json_list_response_entry<0>(itr, row, f);
		compsky::asciify::asciify(itr, ':');
		asciify_json_list_response_entry<1>(itr, row, g);
		compsky::asciify::asciify(itr, ',');
	}
	template<typename Flag,  typename... Args>
	void asciify_json_response_row(char*& itr,  MYSQL_ROW row,  const flag::Dict f_arr_or_dict,  const Flag f,  Args... args){
		asciify_json_list_response_entry<0>(itr, row, f);
		compsky::asciify::asciify(itr, ':');
		constexpr static const flag::Arr _arr;
		compsky::asciify::asciify(itr, '[');
		asciify_json_list_response_entry<1>(itr, row, args...);
		compsky::asciify::asciify(itr, ']');
		compsky::asciify::asciify(itr, ',');
	}
	
	
	template<typename ArrOrDict,  typename... Args>
	void asciify_json_response_rows_from_sql_res(MYSQL_RES* res,  MYSQL_ROW* row,  char*& itr,  const ArrOrDict f_arr_or_dict,  Args... args){
		while(likely((*row = mysql_fetch_row(res))))
			asciify_json_response_row(itr, *row, f_arr_or_dict, args...);
		mysql_free_result(res);
	}
	
	
	template<size_t col_indx>
	size_t strlens(MYSQL_ROW row){
		return 0;
	}
	template<size_t col_indx,  typename... Args>
	size_t strlens(MYSQL_ROW row, const flag::NoQuote,  Args... args);
	template<size_t col_indx,  typename... Args>
	size_t strlens(MYSQL_ROW row, const flag::QuoteNoEscape,  Args... args);
	template<size_t col_indx,  typename... Args>
	size_t strlens(MYSQL_ROW row, const flag::QuoteAndEscape,  Args... args);
	
	template<typename Int>
	size_t strlens_int(Int m){
		size_t n = 1; // Space for the comma
		do{
			++n;
		}while((m /= 10) != 0);
		return n;
	}
	
	template<size_t col_indx,  typename... Args>
	size_t strlens(MYSQL_ROW row, const flag::NoQuote,  Args... args){
		return strlen(row[col_indx]) + 1 /* Trailing comma */ + strlens<col_indx+1>(row, args...);
	}
	
	template<size_t col_indx,  typename... Args>
	size_t strlens(MYSQL_ROW row, const flag::QuoteNoEscape,  Args... args){
		return 1 + strlen(row[col_indx]) + 1 + 1 + strlens<col_indx+1>(row, args...);
	}
	
	template<size_t col_indx,  typename... Args>
	size_t strlens(MYSQL_ROW row, const flag::QuoteAndEscape,  Args... args){
		return 1 + 2*strlen(row[col_indx]) + 1 + 1 + strlens<col_indx+1>(row, args...);
	}
	
	
	
	template<typename... Args>
	size_t get_size_of_json_response_rows_from_sql_res(MYSQL_RES* res,  MYSQL_ROW* row,  Args... args){
		size_t sz = 0;
		while(likely((*row = mysql_fetch_row(res)))){
			sz += 2; // Worst case scenario
			sz += strlens<0>(*row, args...);
		}
		return sz;
	}
} // namespace _r
