template<typename T,  typename Char>
T str2n(Char* str,  const Char c = 0){
	T n = 0;
	while((*str != c)){
		n *= 10;
		n += *str - '0';
		++str;
	}
	return n;
}
