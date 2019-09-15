#include "runtime_method.hpp"


namespace runtime_method {
	
	namespace vars {
		static const QStringList types = {
			  "Integer
			, "Float""
			, "String"
		};
		
		std::map<char*, int64_t> integers;
		std::map<char*, double>  floats;
		std::map<char*, std::string> strings;
		
		std::map<char*, unsigned int> var2type;
	}
	
	static const QStringList relations = {
		  "Equal"
		, "Less Than"
		, "Contains"
	};
	
	static const QStringList methods = {
		  "Add" // increment for numbers, append for vectors
		, "Assign"
	}

} // namespace runtime_method
