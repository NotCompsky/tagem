#pragma once


enum GetRangeHeaderResult {
	none,
	valid,
	invalid
};

constexpr
const char* skip_to_post_data(const char* s){
	while(*(++s) != 0){
		if (*s != '\n')
			continue;
		if (*(++s) == '\r')
			if (*(++s) == '\n')
				return s;
	}
	return nullptr;
}

constexpr
GetRangeHeaderResult get_range(const char* headers,  size_t& from,  size_t& to){
	while(*(++headers) != 0){ // NOTE: headers is guaranteed to be more than 0 characters long, as we have already guaranteed that it starts with the file id
		if (*headers != '\n')
			continue;
		bool is_range_header = false;
		switch(*(++headers)){
			case 'R':
				switch(*(++headers)){
					case 'a':
						switch(*(++headers)){
							case 'n':
								switch(*(++headers)){
									case 'g':
										switch(*(++headers)){
											case 'e':
												switch(*(++headers)){
													case ':':
														switch(*(++headers)){
															case ' ':
																switch(*(++headers)){
																	case 'b':
																		switch(*(++headers)){
																			case 'y':
																				switch(*(++headers)){
																					case 't':
																						switch(*(++headers)){
																							case 'e':
																								switch(*(++headers)){
																									case 's':
																										switch(*(++headers)){
																											case '=':
																												is_range_header = true;
																										}
																										break;
																								}
																								break;
																						}
																						break;
																				}
																				break;
																		}
																		break;
																}
																break;
														}
														break;
												}
												break;
										}
										break;
								}
								break;
						}
						break;
				}
				break;
		}
		if (*headers == 0)
			break;
		if (likely(not is_range_header))
			continue;
		
		++headers; // Skip '=' - a2n is safe even if the next character is null
		from = a2n<size_t>(&headers);
		
		if (*headers != '-')
			return GetRangeHeaderResult::invalid;
		
		++headers; // Skip '-' - a2n is safe even if the next character is null
		to = a2n<size_t>(headers);
		
		return GetRangeHeaderResult::valid;
	}
	from = 0;
	to = 0;
	return GetRangeHeaderResult::none;
}
