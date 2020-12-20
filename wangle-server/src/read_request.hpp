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

#include "str_utils.hpp"
#include "str_parsing_macros.hpp"
#include <compsky/deasciify/a2n.hpp>


enum GetRangeHeaderResult {
	none,
	valid,
	invalid
};


constexpr
GetRangeHeaderResult get_range(const char* str,  size_t& from,  size_t& to){
	while(*(++str) != 0){ // NOTE: str is guaranteed to be more than 0 characters long, as we have already guaranteed that it starts with the file id
		if (*str != '\n')
			continue;
		if (TEST_IS_HEADER(13,"Range: bytes="))
			continue;
		if (*str == 0)
			break;
		
		++str; // Skip '=' - a2n is safe even if the next character is null
		from = a2n<size_t>(&str);
		
		if (*str != '-')
			return GetRangeHeaderResult::invalid;
		
		++str; // Skip '-' - a2n is safe even if the next character is null
		to = a2n<size_t>(str);
		
		return GetRangeHeaderResult::valid;
	}
	from = 0;
	to = 0;
	return GetRangeHeaderResult::none;
}
