inline
const char* basename(const char* s){
	const char* end_of_dirname = s;
	while(*s != 0){
		if (*s == '/')
			end_of_dirname = s;
		++s;
	}
	return end_of_dirname + 1;
}


inline
size_t pardir_length(const char* s){
	return reinterpret_cast<uintptr_t>(basename(s)) - reinterpret_cast<uintptr_t>(s);
}


inline
size_t pardir_length(const QString& s){
	int end_of_dirname__indx = 0;
	for(auto i = 0;  i < s.size();  ){
		if (s.at(i) == QChar('/'))
			end_of_dirname__indx = i;
		++i;
	}
	return end_of_dirname__indx + 1;
}
