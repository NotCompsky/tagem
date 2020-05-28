#pragma once


namespace filetype {
	enum {
		NONE,
		text,
		html,
		image,
		COUNT
	};
	
	int guess(const char* url){
		while(*url != 0)
			++url;
		switch(*(--url)){
			case 'l':
				switch(*(--url)){
					case 'm':
						switch(*(--url)){
							case 't':
								switch(*(--url)){
									case 'h':
										switch(*(--url)){
											case '.':
												return html;
										}
								}
						}
				}
			case 'f':
				switch(*(--url)){
					case 'i':
						switch(*(--url)){
							case 'g':
								switch(*(--url)){
									case '.':
										return image; // gif
								}
						}
				}
			case 'g':
				switch(*(--url)){
					case 'e':
						switch(*(--url)){
							case 'p':
								switch(*(--url)){
									case 'j':
										switch(*(--url)){
											case '.':
												return image; //jpeg
										}
								}
						}
					case 'p':
						switch(*(--url)){
							case 'j':
								switch(*(--url)){
									case '.':
										return image; //jpg
								}
						}
				}
			case 'p':
				switch(*(--url)){
					case 'p':
						switch(*(--url)){
							case 'h':
							case 'c':
								switch(*(--url)){
									case '.':
										return text;
								}
						}
				}
			case 't':
				switch(*(--url)){
					case 'x':
						switch(*(--url)){
							case 't':
								switch(*(--url)){
									case '.':
										return text;
								}
						}
				}
			//case '/':
			//	return html;
		}
		return html;
	}
}
