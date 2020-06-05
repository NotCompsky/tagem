#include "qry.hpp"
#include <compsky/asciify/asciify.hpp>
#include <cstddef> // for size_t

#ifndef NOTTDEBUG
# include <cstdio>
# define LOG(...) fprintf(stderr, ##__VA_ARGS__)
#else
# define LOG(...)
#endif


namespace sql_factory{

namespace arg {
	typedef uint32_t ArgType;
	enum Arg : ArgType {
		invalid,
		file,
		tag,
		tag_tree,
		dir,
		dir_basename,
		order_by,
		order_by_value_asc,
		order_by_value_desc,
		limit,
		value,
		name,
		END_OF_STRING,
		NOT = (1 << 7) // WARNING: Must be no other enums using this bit
	};
}

namespace _f {
	using namespace compsky::asciify::flag;
	constexpr StrLen strlen;
}

struct SQLArg {
	const char* data;
	const size_t len;
};

static
bool has_tag_relation_tbl(const char tbl_alias){
	switch(tbl_alias){
		case 'f':
			return true;
		default:
			return false;
	}
}

static
const char* tbl_full_name(const char tbl_alias){
	switch(tbl_alias){
		case 'f':
			return "file";
		case 'd':
		case 'D':
			return "dir";
		default: // AKA case 't'
			return "tag";
	}
}

static
size_t go_to_next(const char*& qry,  const char c){
	// NOTE: *qry == c
	size_t n = 0;
	while(*(++qry) != c)
		++n;
	return n;
}

template<typename Int>
static
Int s2n(const char*& qry){
	// WARNING: This is a slightly different mechanism to how s2n works in server.cpp - that function does not use a reference.
	Int n = 0;
	while(true){
		n *= 10;
		const char c = *(++qry);
		if ((c >= '0') and (c <= '9'))
			n += c - '0';
		else
			return n / 10;
	}
}

static
successness::ReturnType append_escaped_str(std::string& result,  const char*& qry){
	if (*(++qry) != '"')
		return successness::invalid;
	const char* qry_orig = qry;
	while(true){
		const char c = *(++qry);
		if (*qry == '\\')
			++qry;
		if (*qry == '"')
			break;
		if (*qry == 0)
			return successness::invalid;
	}
	if (*(++qry) != ' ')
		return successness::invalid;
	result.append(qry_orig,  (uintptr_t)qry - (uintptr_t)qry_orig);
	return successness::ok;
}

static
arg::ArgType process_arg(const char*& qry){
	LOG("process_arg %s\n", qry);
	switch(*(++qry)){
		case 0:
			return arg::END_OF_STRING;
		case 'f':
			switch(*(++qry)){
				case ' ':
					return arg::file;
			}
			break;
		case 't':
			switch(*(++qry)){
				case ' ':
					return arg::tag_tree;
			}
			break;
		case 'T':
			switch(*(++qry)){
				case ' ':
					return arg::tag;
			}
			break;
		case 'd':
			switch(*(++qry)){
				case ' ':
					return arg::dir;
			}
			break;
		case 'D':
			switch(*(++qry)){
				case ' ':
					return arg::dir_basename;
			}
			break;
		case 'l':
			switch(*(++qry)){
				case 'i':
					switch(*(++qry)){
						case 'm':
							switch(*(++qry)){
								case 'i':
									switch(*(++qry)){
										case 't':
											switch(*(++qry)){
												case ' ':
													return arg::limit;
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
		case 'n':
			switch(*(++qry)){
				case 'a':
					switch(*(++qry)){
						case 'm':
							switch(*(++qry)){
								case 'e':
									switch(*(++qry)){
										case ' ':
											return arg::name;
									}
									break;
							}
							break;
					}
					break;
				case 'o':
					switch(*(++qry)){
						case 't':
							switch(*(++qry)){
								case ' ':
									return arg::NOT | process_arg(qry);
							}
							break;
					}
					break;
			}
			break;
		case 'o':
			switch(*(++qry)){
				case 'f':
					switch(*(++qry)){
						case 'f':
							switch(*(++qry)){
								case 's':
									switch(*(++qry)){
										case 'e':
											switch(*(++qry)){
												case 't':
													switch(*(++qry)){
														case ' ':
															return arg::offset;
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
				case 'r':
					switch(*(++qry)){
						case 'd':
							switch(*(++qry)){
								case 'e':
									switch(*(++qry)){
										case 'r':
											switch(*(++qry)){
												case ' ':
													return arg::order_by;
												case '-':
													switch(*(++qry)){
														case 'b':
															switch(*(++qry)){
																case 'y':
																	switch(*(++qry)){
																		case '-':
																			switch(*(++qry)){
																				case 'v':
																					switch(*(++qry)){
																						case 'a':
																							switch(*(++qry)){
																								case 'l':
																									switch(*(++qry)){
																										case 'u':
																											switch(*(++qry)){
																												case 'e':
																													switch(*(++qry)){
																														case ' ':
																															switch(*(++qry)){
																																case 'a':
																																	switch(*(++qry)){
																																		case ' ':
																																			return arg::order_by_value_asc;
																																	}
																																	break;
																																case 'd':
																																	switch(*(++qry)){
																																		case ' ':
																																			return arg::order_by_value_desc;
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
									break;
							}
							break;
					}
					break;
			}
			break;
		case 'v':
			switch(*(++qry)){
				case 'a':
					switch(*(++qry)){
						case 'l':
							switch(*(++qry)){
								case 'u':
									switch(*(++qry)){
										case 'e':
											switch(*(++qry)){
												case ' ':
													return arg::value;
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
	return arg::invalid;
}

static
successness::ReturnType process_name_list(std::string& where,  const char tbl_alias,  const char*& qry){
	LOG("process_name_list %c %s\n", tbl_alias, qry);
	const char* qry_begin = qry;
	while(true){
		switch(*(++qry)){
			case '"': {
				const char* start = qry + 1;
				while(true){
					const char c = *(++qry);
					if (c == '\\'){
						// NOTE: Happily, we can see that all names are already escaped - an unescaped double quote inside a single name.
						++qry;
						continue;
					}
					if (c == '"'){
						where += '"';
						where.append(start,  (uintptr_t)qry - (uintptr_t)start);
						where += '"';
						where += ',';
						if (*(++qry) != ' ')
							return successness::invalid;
						break;
					}
					if (c == 0)
						return successness::malicious;
				}
				break;
			}
			default:
				if (qry == qry_begin + 1)
					// then we haven't encountered any names in this list
					return successness::invalid;
				--qry;
				return successness::ok;
		}
	}
}

static
successness::ReturnType process_value_list(std::string& where,  const char*& qry){
	LOG("process_value_list %s\n", qry);
	
	const uint64_t min = s2n<uint64_t>(qry);
	if (*qry != '-')
		return successness::invalid;
	const uint64_t max = s2n<uint64_t>(qry);
	if (*qry != ' ')
		return successness::invalid;
	
	LOG("  min == %lu\n", min);
	LOG("  max == %lu\n", max);
	
	// The following is like the loop in process_name_list, but changing what is done on a match
	// TODO: Deduplicate code
	const char* qry_begin = qry;
	while(true){
		switch(*(++qry)){
			case '"': {
				const char* start = qry + 1;
				while(true){
					const char c = *(++qry);
					if (c == '\\'){
						// NOTE: Happily, we can see that all names are already escaped - an unescaped double quote inside a single name.
						++qry;
						continue;
					}
					if (c == '"'){
						where += "SELECT file FROM file2";
						where.append(start,  (uintptr_t)qry - (uintptr_t)start);
						where += " WHERE x>=";
						where += std::to_string(min);
						where += " AND x<=";
						where += std::to_string(max);
						where += " UNION ";
						if (*(++qry) != ' ')
							return successness::invalid;
						break;
					}
					if (c == 0)
						return successness::malicious;
				}
				break;
			}
			default:
				if (qry == qry_begin + 1)
					// then we haven't encountered any names in this list
					return successness::invalid;
				--qry;
				return successness::ok;
		}
	}
}

static
successness::ReturnType process_order_by_var_name_list(std::string& join,  std::string& order_by,  std::string& order_by_end,  const char*& qry){
	LOG("process_order_by_var_name_list %s\n", qry);
	
	static
	unsigned f2x_indx = 0;
	// This function should only be called once, so static is superfluous
	
	// The following is like the loop in process_name_list, but changing what is done on a match
	// TODO: Deduplicate code
	const char* qry_begin = qry;
	while(true){
		switch(*(++qry)){
			case '"': {
				const char* start = qry + 1;
				while(true){
					const char c = *(++qry);
					if (c == '\\'){
						// NOTE: Happily, we can see that all names are already escaped - an unescaped double quote inside a single name.
						++qry;
						continue;
					}
					if (c == '"'){
						const size_t sz = (uintptr_t)qry - (uintptr_t)start;
						
						if (*(++qry) != ' ')
							return successness::invalid;
						
						join += "LEFT JOIN file2";
						join.append(start, sz);
						join += " f2";
						join += std::to_string(++f2x_indx);
						join += " ON f2";
						join += std::to_string(f2x_indx);
						join += ".file=X.id\n";
						
						order_by += "IFNULL(f2";
						order_by += std::to_string(f2x_indx);
						order_by += ".x,";
						
						order_by_end += ")";
						
						break;
					}
					if (c == 0)
						return successness::malicious;
				}
				break;
			}
			default:
				if (qry == qry_begin + 1)
					// then we haven't encountered any names in this list
					return successness::invalid;
				--qry;
				return successness::ok;
		}
	}
}

static
successness::ReturnType process_args(std::string& join,  std::string& where,  std::string& order_by,  unsigned& limit,  unsigned& offset,  const char which_tbl,  const char* qry){
	LOG("process_args %c %s\n", which_tbl, qry);
	unsigned f2x_indx = 0;
	while(true){
		const arg::ArgType arg_token = process_arg(qry);
		const bool is_inverted = (arg_token & arg::NOT);
		switch(arg_token & ~arg::NOT){ // Ignore the NOT flag
			case arg::END_OF_STRING:
				return successness::ok;
			case arg::invalid:
				return successness::invalid;
			case arg::file:
			case arg::dir: {
				if (which_tbl != 'f')
					return successness::unimplemented;
				where += "\nAND X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN (SELECT f.id FROM file f JOIN dir d ON d.id=f.dir WHERE d.name REGEXP ";
				const auto rc = append_escaped_str(where, qry);
				if (rc != successness::ok)
					return rc;
				where += ")";
				break;
			}
			case arg::dir_basename: {
				if (which_tbl != 'f')
					return successness::unimplemented;
				where += "\nAND X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN (SELECT f.id FROM file f JOIN dir d ON d.id=f.dir WHERE SUBSTR(SUBSTRING_INDEX(d.name, '/', -2), 1, LENGTH(SUBSTRING_INDEX(d.name, '/', -2))-1) REGEXP ";
				const auto rc = append_escaped_str(where, qry);
				if (rc != successness::ok)
					return rc;
				where += ")";
				break;
			}
			case arg::tag: {
				if (not has_tag_relation_tbl(which_tbl))
					return successness::invalid;
				where += "\nAND X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN (SELECT x2t.";
				where += tbl_full_name(which_tbl);
				where += "_id FROM ";
				where += tbl_full_name(which_tbl);
				where += "2tag x2t JOIN tag t ON t.id=x2t.tag_id WHERE t.name IN (";
				const auto rc = process_name_list(where, 't', qry);
				if (rc != successness::ok)
					return rc;
				where.pop_back(); // Remove trailing comma (which is guaranteed to exist)
				where += "))";
				break;
			}
			case arg::tag_tree: {
				if (not has_tag_relation_tbl(which_tbl))
					return successness::invalid;
				where += "\nAND X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN (SELECT x2t.";
				where += tbl_full_name(which_tbl);
				where += "_id FROM ";
				where += tbl_full_name(which_tbl);
				where += "2tag x2t JOIN tag2parent_tree t2pt ON t2pt.tag=x2t.tag_id JOIN tag t ON t.id=t2pt.parent WHERE t.name IN (";
				const auto rc = process_name_list(where, 't', qry);
				if (rc != successness::ok)
					return rc;
				where.pop_back(); // Remove trailing comma (which is guaranteed to exist)
				where += "))";
				break;
			}
			case arg::order_by: {
				if ((not order_by.empty()) or (*(++qry) != '"') or is_inverted)
					return successness::invalid;
				const size_t order_by_sz = go_to_next(qry, '"');
				order_by = std::string(qry + 1,  order_by_sz);
				if (*(++qry) != ' ')
					return successness::invalid;
				break;
			}
			case arg::order_by_value_asc:
			case arg::order_by_value_desc: {
				if ((not order_by.empty()) or is_inverted)
					return successness::invalid;
				
				std::string order_by_end;
				const auto rc = process_order_by_var_name_list(join, order_by, order_by_end, qry);
				if (rc != successness::ok)
					return rc;
				
				order_by += "NULL";
				order_by += order_by_end + ((arg_token==arg::order_by_value_asc)?" ASC":"DESC");
				
				break;
			}
			case arg::value: {
				where += "\nAND X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN (";
				const auto rc = process_value_list(where, qry);
				if (rc != successness::ok)
					return rc;
				for (auto i = 0;  i < 7;  ++i)
					// Strip the trailing " UNION "
					where.pop_back();
				where += ")";
				break;
			}
			case arg::name: {
				where += "\nAND X.name ";
				if (is_inverted)
					where += "NOT ";
				where += "REGEXP ";
				const auto rc = append_escaped_str(where, qry);
				if (rc != successness::ok)
					return rc;
				break;
			}
			case arg::limit: {
				limit = s2n<unsigned>(qry);
				if (*qry != ' ')
					return successness::invalid;
				if (limit > 100)
					limit = 100;
				break;
			}
			case arg::offset: {
				offset = s2n<unsigned>(qry);
				if (*qry != ' ')
					return successness::invalid;
				break;
			}
		}
	}
}

successness::ReturnType parse_into(char* itr,  const char* qry){
	const char* const buf = itr;
	
	char which_tbl;
	std::string join = "";
	std::string where = "";
	std::string order_by = "";
	unsigned limit = 10;
	unsigned offset = 0;
	where.reserve(4096);
	switch(*qry){
		case 'f':
		case 'd':
		case 't':
			which_tbl = *qry;
			break;
		default:
			return successness::invalid;
	}
	switch(*(++qry)){
		case ' ': break;
		default: return successness::invalid;
	}
	
	const auto rc = process_args(join, where, order_by, limit, offset, which_tbl, qry);
	if (rc != successness::ok){
		LOG("join == %s\n", join.c_str());
		LOG("where == %s\n", where.c_str());
		LOG("order_by == %s\n", order_by.c_str());
		return rc;
	}
	
	compsky::asciify::asciify(
		itr,
		"SELECT "
			"X.id\n"
		"FROM ", tbl_full_name(which_tbl), " X\n",
		join.c_str(),
		"WHERE TRUE", where.c_str(),
		(order_by.empty())?"":"\nORDER BY ", order_by.c_str(), "\n"
		"OFFSET ", offset, " "
		"LIMIT ", limit,
		'\0'
	);
	
	LOG("Query OK\n");
	
	return successness::ok;
}

} // namespace sql_factory
