#pragma once


namespace _method {
	enum {
		GET,
		POST,
		UNKNOWN
	};
}

constexpr
unsigned int which_method(const char*& s){
	switch(*(s++)){
		case 'P':
			switch(*(s++)){
				case 'O':
					switch(*(s++)){
						case 'S':
							switch(*(s++)){
								case 'T':
									switch(*(s++)){
										case ' ':
											return _method::POST;
									}
									break;
							}
							break;
					}
					break;
			}
			break;
		case 'G':
			switch(*(s++)){
				case 'E':
					switch(*(s++)){
						case 'T':
							switch(*(s++)){
								case ' ':
									return _method::GET;
							}
							break;
					}
					break;
			}
			break;
	}
	return _method::UNKNOWN;
}
