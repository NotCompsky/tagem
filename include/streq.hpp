inline
bool streq(const char* a,  const char* b){
	while(*a == *b){
		++a;
		++b;
	}
	return (*a == 0  and  *b == 0);
}
