inline
bool streq(const char* a,  const char* b){
	while(*a != 0){
		if (*a != *b)
			return false;
		++a;
		++b;
	}
	return true;
}
