#pragma once

#include <QStringList>


namespace runtime_method {

	namespace vars {
		enum {
			  integer
			, floating_point
			, string
		};
		
		extern std::map<char*, int64_t> integers;
		extern std::map<char*, double>  floats;
		extern std::map<char*, std::string> strings;
		
		extern std::map<char*, unsigned int> var2type;
	}
	
	namespace relation {
		enum {
			  eq
			, lt
			, contains
		};
	}
	
	namespace method {
		enum {
			  add
			, assign
		};
	}

} // namespace runtime_method
