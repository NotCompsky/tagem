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

namespace attribute_kind {
	enum AttributeKind {
		unique,
		one_to_one,
		many_to_many
	};
}

namespace attribute_value_kind {
	enum AttributeValueKind {
		integer,
		string
	};
}

namespace arg {
	typedef uint32_t ArgType;
	enum Arg : ArgType {
		invalid,
		file,
		tag,
		tag_tree,
		dir,
		dir_basename,
		backups,
		external_db,
		order_by_asc,
		order_by_desc,
		order_by_value_asc,
		order_by_value_desc,
		limit,
		offset,
		value,
		attribute,
		same_attribute,
		name,
		bracket_open,
		bracket_close,
		
		operator_and,
		operator_or,
		
		END_OF_STRING,
		NOT = (1 << 30) // WARNING: Must be no other enums using this bit
	};
}

namespace attribute_name {
	constexpr static const char* const DCT_HASH = "dct_hash";
	constexpr static const char* const DIR = "dir";
	constexpr static const char* const DURATION = "duration";
	constexpr static const char* const NAME = "name";
	constexpr static const char* const TAG = "tag";
	constexpr static const char* const ID = "id";
	constexpr static const char* const AUDIO_HASH = "audio_hash";
	constexpr static const char* const MD5 = "md5";
	constexpr static const char* const MIMETYPE = "mimetype";
	constexpr static const char* const SHA256 = "sha256";
	constexpr static const char* const SIZE = "size";
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
const char* tbl_full_name_of_base_tbl(const char tbl_alias){
	switch(tbl_alias){
		case 'f':
			return "_file";
		case 'd':
		case 'D':
			return "_dir";
		default: // AKA case 't'
			return "_tag";
	}
}

static
const char* attribute_field_name(const char* const attribute_name){
	// NOTE: Thus far only used for many-to-many variables
	// Compiler refuses to allow the switch operator
	if ((attribute_name == attribute_name::DCT_HASH) or (attribute_name == attribute_name::AUDIO_HASH))
		return "x";
	else // attribute_name::TAG
		return "tag";
}

static
attribute_value_kind::AttributeValueKind get_attribute_value_kind(const char* const attribute_name){
	if ((attribute_name == attribute_name::DIR) or (attribute_name == attribute_name::DURATION) or (attribute_name == attribute_name::ID) or (attribute_name == attribute_name::SIZE))
		return attribute_value_kind::integer;
	else // attribute_name::NAME
		return attribute_value_kind::string;
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

template<typename Int>
successness::ReturnType get_int_range(const char*& qry,  Int& min,  Int& max){
	min = s2n<uint64_t>(qry);
	if (*qry != '-')
		return successness::invalid;
	max = s2n<uint64_t>(qry);
	if (*qry == '?'){
		max = -1;
		++qry;
	}
	if (*qry != ' ')
		return successness::invalid;
	
	return successness::ok;
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
		
		case '(':
			switch(*(++qry)){
				case ' ':
					return arg::bracket_open;
			}
			break;
		case ')':
			switch(*(++qry)){
				case ' ':
					return arg::bracket_close;
			}
			break;
		
		case '"':
			return arg::name;
		
		case 'a':
			switch(*(++qry)){
				case 'n':
					switch(*(++qry)){
						case 'd':
							switch(*(++qry)){
								case ' ':
									return arg::operator_and;
							}
							break;
					}
					break;
				case 't':
					switch(*(++qry)){
						case 't':
							switch(*(++qry)){
								case 'r':
									switch(*(++qry)){
										case ' ':
											return arg::attribute;
									}
									break;
							}
							break;
					}
					break;
			}
			break;
		case 'b':
			switch(*(++qry)){
				case 'a':
					switch(*(++qry)){
						case 'c':
							switch(*(++qry)){
								case 'k':
									switch(*(++qry)){
										case 'u':
											switch(*(++qry)){
												case 'p':
													switch(*(++qry)){
														case 's':
															switch(*(++qry)){
																case ' ':
																	return arg::backups;
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
						case ' ':
							return arg::operator_or;
						case 'd':
							switch(*(++qry)){
								case 'e':
									switch(*(++qry)){
										case 'r':
											switch(*(++qry)){
												case ' ':
													switch(*(++qry)){
														case 'a':
															switch(*(++qry)){
																case ' ':
																	return arg::order_by_asc;
															}
															break;
														case 'd':
															switch(*(++qry)){
																case ' ':
																	return arg::order_by_desc;
															}
															break;
													}
													break;
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
		case 's':
			switch(*(++qry)){
				case 'a':
					switch(*(++qry)){
						case 'm':
							switch(*(++qry)){
								case 'e':
									switch(*(++qry)){
										case ' ':
											return arg::same_attribute;
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
		case 'x':
			switch(*(++qry)){
				case ' ':
					return arg::external_db;
			}
			break;
	}
	return arg::invalid;
}

static
successness::ReturnType get_attribute_name(const char which_tbl,  const char*& qry,  const char*& attribute_name,  attribute_kind::AttributeKind& attribute_kind){
	switch(*(++qry)){
		case 'a':
			switch(*(++qry)){
				case 'u':
					switch(*(++qry)){
						case 'd':
							switch(*(++qry)){
								case 'i':
									switch(*(++qry)){
										case 'o':
											switch(*(++qry)){
												case ' ':
													attribute_name = attribute_name::AUDIO_HASH;
													attribute_kind = attribute_kind::many_to_many;
													return (which_tbl=='f') ? successness::ok : successness::invalid;
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
		case 'd':
			switch(*(++qry)){
				case 'c':
					switch(*(++qry)){
						case 't':
							switch(*(++qry)){
								case ' ':
									attribute_name = attribute_name::DCT_HASH;
									attribute_kind = attribute_kind::many_to_many;
									return (which_tbl=='f') ? successness::ok : successness::invalid;
							}
							break;
					}
					break;
				case 'i':
					switch(*(++qry)){
						case 'r':
							switch(*(++qry)){
								case ' ':
									attribute_name = attribute_name::DIR;
									attribute_kind = attribute_kind::one_to_one;
									return (which_tbl=='f') ? successness::ok : successness::invalid;
							}
							break;
					}
					break;
				case 'u':
					switch(*(++qry)){
						case 'r':
							switch(*(++qry)){
								case 'a':
									switch(*(++qry)){
										case 't':
											switch(*(++qry)){
												case 'i':
													switch(*(++qry)){
														case 'o':
															switch(*(++qry)){
																case 'n':
																	switch(*(++qry)){
																		case ' ':
																			// same duration
																			attribute_name = attribute_name::DURATION;
																			attribute_kind = attribute_kind::one_to_one;
																			return (which_tbl=='f') ? successness::ok : successness::invalid;
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
		case 'i':
			switch(*(++qry)){
				case 'd':
					switch(*(++qry)){
						case ' ':
							attribute_name = attribute_name::ID;
							attribute_kind = attribute_kind::unique;
							return successness::ok;
					}
					break;
			}
			break;
		case 'm':
			switch(*(++qry)){
				case 'd':
					switch(*(++qry)){
						case '5':
							switch(*(++qry)){
								case ' ':
									// same md5
									attribute_name = attribute_name::MD5;
									attribute_kind = attribute_kind::one_to_one;
									return (which_tbl=='f') ? successness::ok : successness::invalid;
							}
							break;
					}
					break;
				case 'i':
					switch(*(++qry)){
						case 'm':
							switch(*(++qry)){
								case 'e':
									switch(*(++qry)){
										case ' ':
											// same mime
											attribute_name = attribute_name::MIMETYPE;
											attribute_kind = attribute_kind::one_to_one;
											return (which_tbl=='f') ? successness::ok : successness::invalid;
									}
									break;
							}
							break;
					}
					break;
			}
			break;
		case 's':
			switch(*(++qry)){
				case 'h':
					switch(*(++qry)){
						case 'a':
							switch(*(++qry)){
								case ' ':
									attribute_name = attribute_name::SHA256;
									attribute_kind = attribute_kind::one_to_one;
									return (which_tbl=='f') ? successness::ok : successness::invalid;
							}
							break;
					}
					break;
				case 'i':
					switch(*(++qry)){
						case 'z':
							switch(*(++qry)){
								case 'e':
									switch(*(++qry)){
										case ' ':
											attribute_name = attribute_name::SIZE;
											attribute_kind = attribute_kind::one_to_one;
											return (which_tbl=='f') ? successness::ok : successness::invalid;
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
											attribute_name = attribute_name::NAME;
											attribute_kind = attribute_kind::one_to_one;
											return successness::ok;
									}
									break;
							}
							break;
					}
					break;
			}
			break;
		case 't':
			switch(*(++qry)){
				case 'a':
					switch(*(++qry)){
						case 'g':
							switch(*(++qry)){
								case ' ':
									attribute_name = attribute_name::TAG;
									attribute_kind = attribute_kind::many_to_many;
									return (which_tbl=='f') ? successness::ok : successness::invalid;
							}
							break;
					}
					break;
			}
			break;
	}
	return successness::invalid;
}

static
successness::ReturnType process_name_list(std::string& where,  const char tbl_alias,  const char*& qry){
	LOG("process_name_list %c %s\n", tbl_alias, qry);
	const size_t where_length_orig = where.size();
	where += " REGEXP "; // This is overwritten with " IN (   " if there is only one value in this list
	const char* qry_begin = qry;
	unsigned n_elements = 0;
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
						++n_elements;
						break;
					}
					if (c == 0)
						return successness::malicious;
				}
				break;
			}
			default:
				if (n_elements == 0)
					// then we haven't encountered any names in this list
					return successness::invalid;
				--qry;
				where.pop_back(); // Remove trailing comma (which is guaranteed to exist)
				if (n_elements == 1){
					where.replace(where_length_orig, 8, " IN (   ");
					where += ")";
				}
				return successness::ok;
		}
	}
}

static
successness::ReturnType process_value_list(std::string& where,  const char*& qry){
	LOG("process_value_list %s\n", qry);
	
	uint64_t min;
	uint64_t max;
	const auto rc = get_int_range(qry, min, max);
	if (rc != successness::ok)
		return rc;
	
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
successness::ReturnType process_args(const char* const user_disallowed_X_tbl_filter_inner_pre,  const unsigned user_id,  std::string& join,  std::string& where,  std::string& order_by,  unsigned& limit,  unsigned& offset,  const char which_tbl,  const char* qry){
	LOG("process_args %c %s\n", which_tbl, qry);
	unsigned f2x_indx = 0;
	constexpr size_t max_bracket_depth = 16; // Arbitrary limit
	constexpr static const char* const _operator_or  = "\nOR";
	constexpr static const char* const _operator_and = "\nAND";
	constexpr static const char* const _operator_none = "";
	const char* bracket_operator_at_depth[max_bracket_depth] = {_operator_none};
	unsigned bracket_depth = 0;
	int n_args_since_operator = 0;
	std::string join_for_auto_ordering = "";
	std::string auto_order_by = "";
	
	// Some arguments must only be encountered at most once
	unsigned n_calls__order_by = 0;
	unsigned n_calls__limit = 0;  // Doesn't reallly matter to the serverif multiple limits are specified
	unsigned n_calls__offset = 0; // Doesn't reallly matter to the serverif multiple offests are specified
	unsigned n_calls__backups = 0;
	
	const char* attribute_name;
	attribute_kind::AttributeKind attribute_kind;
	unsigned min;
	unsigned max;
	successness::ReturnType rc;
	
	while(true){
		const arg::ArgType arg_token = process_arg(qry);
		const bool is_inverted = (arg_token & arg::NOT);
		const arg::ArgType arg_token_base = arg_token & ~arg::NOT;
		switch(arg_token_base){ // Ignore the NOT flag
			case arg::END_OF_STRING:
				if (join.empty())
					join = join_for_auto_ordering;
				if (order_by.empty())
					order_by = auto_order_by;
				return ((bracket_depth == 0) and (n_args_since_operator == 1)) ? successness::ok : successness::invalid;
				// WARNING: This also disallows using no filters altogether.
				// TODO: Fix that.
			case arg::invalid:
				return successness::invalid;
			
			case arg::operator_or:
			case arg::operator_and:
				if (n_args_since_operator != 1)
					return successness::invalid;
				bracket_operator_at_depth[bracket_depth] = ((arg_token_base) == arg::operator_or) ? _operator_or : _operator_and;
				n_args_since_operator = 0;
				break;
			
			case arg::bracket_open:
				where += bracket_operator_at_depth[bracket_depth];
				where += "\n(";
				++bracket_depth;
				bracket_operator_at_depth[bracket_depth] = _operator_none;
				break;
			case arg::bracket_close:
				where += ")";
				--bracket_depth;
				break;
			
			case arg::file:
			case arg::dir: {
				if (which_tbl != 'f')
					return successness::unimplemented;
				where += bracket_operator_at_depth[bracket_depth];
				where += " X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN (SELECT f.id FROM _file f JOIN _dir d ON d.id=f.dir WHERE d.name REGEXP ";
				rc = append_escaped_str(where, qry);
				if (rc != successness::ok)
					return rc;
				where += ")";
				++n_args_since_operator;
				break;
			}
			case arg::dir_basename: {
				if (which_tbl != 'f')
					return successness::unimplemented;
				where += bracket_operator_at_depth[bracket_depth];
				where += " X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN (SELECT f.id FROM _file f JOIN _dir d ON d.id=f.dir WHERE SUBSTR(SUBSTRING_INDEX(d.name, '/', -2), 1, LENGTH(SUBSTRING_INDEX(d.name, '/', -2))-1) REGEXP ";
				rc = append_escaped_str(where, qry);
				if (rc != successness::ok)
					return rc;
				where += ")";
				++n_args_since_operator;
				break;
			}
			case arg::tag: {
				if (not has_tag_relation_tbl(which_tbl))
					return successness::invalid;
				where += bracket_operator_at_depth[bracket_depth];
				where += " X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN (SELECT x2t.";
				where += tbl_full_name(which_tbl);
				where += " FROM ";
				where += tbl_full_name(which_tbl);
				where += "2tag x2t JOIN _tag t ON t.id=x2t.tag WHERE t.name";
				rc = process_name_list(where, 't', qry);
				if (rc != successness::ok)
					return rc;
				where += ")";
				++n_args_since_operator;
				break;
			}
			case arg::tag_tree: {
				if (not has_tag_relation_tbl(which_tbl))
					return successness::invalid;
				where += bracket_operator_at_depth[bracket_depth];
				where += " X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN (SELECT x2t.";
				where += tbl_full_name(which_tbl);
				where += " FROM ";
				where += tbl_full_name(which_tbl);
				where += "2tag x2t JOIN tag2parent_tree t2pt ON t2pt.tag=x2t.tag JOIN _tag t ON t.id=t2pt.parent WHERE t.name";
				rc = process_name_list(where, 't', qry);
				if (rc != successness::ok)
					return rc;
				where += ")";
				++n_args_since_operator;
				break;
			}
			case arg::backups: {
				if ((++n_calls__backups == 2) or (which_tbl != 'f'))
					return successness::invalid;
				
				rc = get_int_range(qry, min, max);
				if (rc != successness::ok)
					return rc;
				
				where += bracket_operator_at_depth[bracket_depth];
				where += " X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN(SELECT file FROM file_backup GROUP BY file HAVING COUNT(*)>=";
				where += std::to_string(min);
				where += " AND COUNT(*)<=";
				where += std::to_string(max);
				where += ")";
				++n_args_since_operator;
				break;
			}
			case arg::external_db: {
				if (which_tbl != 'f')
					return successness::invalid;
				where += bracket_operator_at_depth[bracket_depth];
				where += " X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN (SELECT f2p.file FROM file2post f2p JOIN external_db db ON db.id=f2p.db WHERE db.name";
				rc = process_name_list(where, 'x', qry);
				if (rc != successness::ok)
					return rc;
				where += ")";
				++n_args_since_operator;
				break;
			}
			
			case arg::order_by_asc:
			case arg::order_by_desc: {
				if ((++n_calls__order_by == 2) or (not order_by.empty()) or is_inverted)
					return successness::invalid;
				
				rc = get_attribute_name(which_tbl, qry, attribute_name, attribute_kind);
				if (rc != successness::ok)
					return rc;
				
				if(attribute_kind == attribute_kind::many_to_many){
					join += "JOIN file2";
					join += attribute_name;
					join += " f2_join ON f2_join.file=X.id\n";
					order_by = "f2_join.";
					order_by += attribute_field_name(attribute_name);
					order_by += (arg_token==arg::order_by_asc)?" ASC":" DESC";
				} else {
					order_by = std::string(attribute_name) + ((arg_token==arg::order_by_asc)?" ASC":" DESC");
				}
				break;
			}
			case arg::order_by_value_asc:
			case arg::order_by_value_desc: {
				if ((++n_calls__order_by == 2) or (not order_by.empty()) or is_inverted)
					return successness::invalid;
				
				std::string order_by_end;
				rc = process_order_by_var_name_list(join, order_by, order_by_end, qry);
				if (rc != successness::ok)
					return rc;
				
				order_by += "NULL";
				order_by += order_by_end + ((arg_token==arg::order_by_value_asc)?" ASC":"DESC");
				
				break;
			}
			case arg::value: {
				where += bracket_operator_at_depth[bracket_depth];
				where += " X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN (";
				rc = process_value_list(where, qry);
				if (rc != successness::ok)
					return rc;
				for (auto i = 0;  i < 7;  ++i)
					// Strip the trailing " UNION "
					where.pop_back();
				where += ")";
				++n_args_since_operator;
				break;
			}
			case arg::same_attribute: {
				if (which_tbl != 'f')
					return successness::invalid;
				auto rc = get_attribute_name(which_tbl, qry, attribute_name, attribute_kind);
				if (rc != successness::ok)
					return rc;
				
				rc = get_int_range(qry, min, max);
				if (rc != successness::ok)
					return rc;
				
				const std::string old_where = where;
				if (attribute_kind == attribute_kind::unique)
					return successness::invalid;
				if (attribute_kind == attribute_kind::one_to_one){
					where = "X.";
					where += attribute_name;
					if (is_inverted)
						where += " NOT";
					where += " IN(SELECT X.";
					where += attribute_name;
					where += " AS x\nFROM ";
					where += tbl_full_name_of_base_tbl(which_tbl);
					where += " X\n";
					where += join;
					where += "WHERE ";
					where += (old_where.empty())?"TRUE":old_where;
					where += "\nAND X.id NOT IN(";
					where += user_disallowed_X_tbl_filter_inner_pre;
					where += std::to_string(user_id);
					where += ")GROUP BY ";
					where += attribute_name;
					where += "\nHAVING COUNT(*)>=";
					where += std::to_string(min);
					where += " AND COUNT(*)<=";
					where += std::to_string(max);
					where += ")";
				} else {
					where = "X.id";
					if (is_inverted)
						where += "NOT";
					where += " IN(SELECT file\n\tFROM file2";
					where += attribute_name;
					where += "\n\tWHERE ";
					where += attribute_field_name(attribute_name);
					where += " IN";
					
					std::string derived_tbl = "";
					derived_tbl += "(\n\t\tSELECT ";
					derived_tbl += attribute_field_name(attribute_name);
					derived_tbl += " AS x\n\t\tFROM file2";
					derived_tbl += attribute_name;
					derived_tbl += " f2_\n\t\tJOIN ";
					derived_tbl += tbl_full_name_of_base_tbl(which_tbl);
					derived_tbl += " X ON X.id=f2_.file\n\t\t";
					derived_tbl += join;
					derived_tbl += "\n\t\tWHERE ";
					derived_tbl += (old_where.empty())?"TRUE":old_where;
					derived_tbl += "\n\t\tAND X.id NOT IN(";
					derived_tbl += user_disallowed_X_tbl_filter_inner_pre;
					derived_tbl += std::to_string(user_id);
					derived_tbl += ")\n\t\tGROUP BY x";
					derived_tbl += "\nHAVING COUNT(*)>=";
					derived_tbl += std::to_string(min);
					derived_tbl += " AND COUNT(*)<=";
					derived_tbl += std::to_string(max);
					derived_tbl += "\n\t)";
					where += derived_tbl;
					where += "\n)";
					join_for_auto_ordering = "JOIN file2" + std::string(attribute_name) + " f2_order ON f2_order.file=X.id\nJOIN " + derived_tbl + " f2_derived ON f2_order.x=f2_derived.x\n";
					auto_order_by = "f2_order.x DESC";
				}
				join = "";
				
				bracket_depth = 0;
				n_args_since_operator = 1;
				
				break;
			}
			case arg::name:
			case arg::attribute: {
				char comparison_mode;
				if (arg_token_base == arg::name){
					attribute_name = attribute_name::NAME;
					attribute_kind = attribute_kind::unique;
					comparison_mode = 'r';
				} else {
					rc = get_attribute_name(which_tbl, qry, attribute_name, attribute_kind);
					if (rc != successness::ok)
						return rc;
					if (attribute_kind == attribute_kind::many_to_many)
						return successness::invalid;
					
					comparison_mode = *(++qry);
					if (not ((comparison_mode == '>') or (comparison_mode == '<') or (comparison_mode == '=') or (comparison_mode == 'r')) or (*(++qry) != ' '))
						return successness::invalid;
				}
				
				where += bracket_operator_at_depth[bracket_depth];
				if (is_inverted)
					where += " NOT";
				where += " X.";
				where += attribute_name;
				
				switch(get_attribute_value_kind(attribute_name)){
					case attribute_value_kind::integer: {
						if (comparison_mode == 'r')
							return successness::invalid;
						where += comparison_mode;
						const uint64_t n = s2n<uint64_t>(qry);
						if (n == 0)
							return successness::invalid;
						where += std::to_string(n);
						break;
					}
					default: // string
						switch(comparison_mode){
							case 'r':
								where += " REGEXP ";
								break;
							case '=':
								where += '=';
								break;
							default:
								return successness::invalid;
						}
						rc = append_escaped_str(where, qry);
						if (rc != successness::ok)
							return rc;
				}
				
				++n_args_since_operator;
				break;
			}
			case arg::limit: {
				if (++n_calls__limit == 2)
					return successness::invalid;
				
				limit = s2n<unsigned>(qry);
				if (*qry != ' ')
					return successness::invalid;
				if (limit > 100)
					limit = 100;
				break;
			}
			case arg::offset: {
				if (++n_calls__offset == 2)
					return successness::invalid;
				
				offset = s2n<unsigned>(qry);
				if (*qry != ' ')
					return successness::invalid;
				break;
			}
		}
	}
}

successness::ReturnType parse_into(char* itr,  const char* qry,  const unsigned user_id){
	// TODO: Look into filtering the filters with the user_id permission filters, to avoid brute-forcing.
	// Not sure this is a big issue.
	
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
	
	const char* user_disallowed_X_tbl_filter_inner_pre;
	switch(which_tbl){
		case 'f':
			user_disallowed_X_tbl_filter_inner_pre = USER_DISALLOWED_FILES_INNER_PRE;
			break;
		case 'd':
			user_disallowed_X_tbl_filter_inner_pre = USER_DISALLOWED_DIRS_INNER_PRE;
			break;
		case 't':
			user_disallowed_X_tbl_filter_inner_pre = USER_DISALLOWED_TAGS_INNER_PRE;
			break;
		// No other values are possible
	}
	
	const auto rc = process_args(user_disallowed_X_tbl_filter_inner_pre, user_id, join, where, order_by, limit, offset, which_tbl, qry);
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
		"FROM ", tbl_full_name_of_base_tbl(which_tbl), " X\n",
		join.c_str(),
		"WHERE ", where.c_str(), "\n"
		  "AND X.id NOT IN(", user_disallowed_X_tbl_filter_inner_pre, user_id, ")"
		"ORDER BY ", ((order_by.empty()) ? "NULL" : order_by.c_str()), "\n"
		"LIMIT ", limit, " "
		"OFFSET ", offset,
		'\0'
	);
	
	LOG("Query OK\n");
	
	return successness::ok;
}

} // namespace sql_factory
