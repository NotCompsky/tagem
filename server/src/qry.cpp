/*
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
*/
#include "qry.hpp"
#include <compsky/asciify/asciify.hpp>
#include <compsky/macros/is_contained_in_varargs.hpp>
#include <compsky/utils/ptrdiff.hpp>
#include <cstddef> // for size_t

#include <boost/preprocessor/seq/for_each.hpp>

#include "log.hpp"

#define ARGTYPE_IN(x,ls) IS_CONTAINED_IN_VARARGS(arg::ArgType,x,ls)
#define CHAR_IN(x,ls) IS_CONTAINED_IN_VARARGS(char,x,ls)
#define STR_IN(x,ls) IS_CONTAINED_IN_VARARGS(const char*,x,ls)
#define MAX_LIMIT 100


namespace sql_factory{

namespace attribute_kind {
	enum AttributeKind {
		unique,
		many_to_one,
		ersatz_many_to_one,
		many_to_many,
		global_fn
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
		era,
		file,
		tag,
		tag_tree,
		dir,
		dir_basename,
		device_exists,
		eras,
		backups,
		boxes,
		tags,
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
		
		select_count,
		select_list,
		select_url_and_title__markdown,
		select_total_size,
		select_total_views,
		select_check_local_files,
		select_delete_local_files,
		select_export_results,
		
		END_OF_STRING,
		NOT = (1 << 30) // WARNING: Must be no other enums using this bit
	};
}

namespace attribute_name {
	constexpr static const char* const DCT_HASH = "dct_hash";
	constexpr static const char* const DIR = "dir";
	constexpr static const char* const DURATION = "duration";
	constexpr static const char* const NAME = "name";
	constexpr static const char* const TITLE = "title";
	constexpr static const char* const DESCRIPTION = "description";
	constexpr static const char* const TAG = "tag";
	constexpr static const char* const ID = "id";
	constexpr static const char* const AUDIO_HASH = "audio_hash";
	constexpr static const char* const MD5 = "md5";
	constexpr static const char* const MD5_OF_PATH = "md5_of_path";
	constexpr static const char* const MIMETYPE = "mimetype";
	constexpr static const char* const SHA256 = "sha256";
	constexpr static const char* const THUMBNAIL = "thumbnail";
	constexpr static const char* const SIZE = "size";
	constexpr static const char* const STATUS = "status";
	constexpr static const char* const ERSATZ_SIZE = "n";
	constexpr static const char* const VIEWS    = "views";
	constexpr static const char* const LIKES    = "likes";
	constexpr static const char* const DISLIKES = "dislikes";
	constexpr static const char* const WIDTH  = "w";
	constexpr static const char* const HEIGHT = "w";
	constexpr static const char* const T_ORIGIN   = "t_origin";
	constexpr static const char* const T_RECORDED = "added_on";
	constexpr static const char* const T_ENDED = "t_ended";
	constexpr static const char* const FPS = "fps";
	constexpr static const char* const PARENT = "parent";
	constexpr static const char PARENTY[] = "parent"; // Array style to guarantee different address
	constexpr static const char* const CHILD = "id";
	constexpr static const char CHILDY[] = "id";
}

namespace selected_field {
	// Array styles used to guarantee unique addresses
	constexpr const char x_id[] = "X.id";
	constexpr const char* count = "COUNT(*)";
	constexpr const char* list__file = "CONCAT(d.full_path, X.name)";
	constexpr const char* list = "X.name";
	constexpr const char* url_and_title__markdown = "CONCAT(d.full_path, X.name), IFNULL(X.title,X.name)";
	constexpr const char* total_size = "SUM(IFNULL(X.size,0))";
	constexpr const char* total_views = "SUM(IFNULL(X.views,0))";
	constexpr const char check_local_files[]  = "CONCAT(d.full_path, X.name)";
	constexpr const char delete_local_files[] = "X.id, CONCAT(d.full_path, X.name)";
	constexpr const char export_results[] = "X.id";
	selected_field::Type get_enum(const char* const str){
		return
		  (str == selected_field::x_id)  ? selected_field::X_ID
		: (str == selected_field::count) ? selected_field::COUNT
		: (str == selected_field::total_size)  ? selected_field::TOTAL_SIZE
		: (str == selected_field::total_views) ? selected_field::TOTAL_VIEWS
		: (str == selected_field::check_local_files)  ? selected_field::CHECK_LOCAL_FILES
		: (str == selected_field::delete_local_files) ? selected_field::DELETE_LOCAL_FILES
		: (str == selected_field::url_and_title__markdown) ? selected_field::URL_AND_TITLE__MARKDOWN
		: (str == selected_field::export_results) ? selected_field::EXPORT_RESULTS
		: selected_field::LIST
		;
	}
};

namespace _f {
	using namespace compsky::asciify::flag;
	constexpr StrLen strlen;
}

struct SQLArg {
	const char* data;
	const size_t len;
};

static
const char* get_tbl_for_file_assoc_count(const arg::ArgType arg){
	switch(arg){
		case arg::eras:
			return "eras";
		case arg::backups:
			return "file_backup";
		case arg::boxes:
			return "box";
		case arg::tags:
			return "file2tag";
		default:
			abort();
	}
}

static
char has_name_attribute(const char tbl_alias){
	switch(tbl_alias){
		case 'e':
			return false;
	}
	return true;
}

static
const char* tbl_full_name(const char tbl_alias){
	switch(tbl_alias){
		case 'e':
			return "era";
		case 'f':
			return "file";
		case 'd':
			return "dir";
		case 't':
			return "tag";
		default:
			abort();
	}
}

static
char tbl_arg_to_alias(const arg::ArgType arg){
	switch(arg){
		case arg::era:
			return 'e';
		case arg::file:
			return 'f';
		case arg::dir:
		case arg::dir_basename:
			return 'd';
		case arg::tag:
		case arg::tag_tree:
			return 't';
		default:
			abort();
	}
}

static
const char* attribute_field_name(const char* const attribute_name){
	// NOTE: Thus far only used for many-to-many variables
	// Compiler refuses to allow the switch operator
	if (STR_IN(attribute_name,(attribute_name::DCT_HASH)(attribute_name::AUDIO_HASH)))
		return "x";
	else
		return attribute_name;
}

static
const char* get_tbl_name_of_join_in_ersatz_many_to_one(const char* const attribute_name,  const char which_tbl){
	//if (attribute_name == attribute_name::ERSATZ_SIZE){
		// Unnecessary atm
		switch(which_tbl){
			case 't':
				return "file2tag";
			case 'd':
				return "file";
			default:
				// i.e. case 'D':
				return "dir";
		}
	//}
}

static
void add_many2many_join_tbl_name(std::string& join,  const char* const attribute_name,  const char which_tbl){
	switch(which_tbl){
		case 'f':
			join += "file2";
			join += attribute_name;
			return;
		case 't':
			join += "tag2parent";
			if (STR_IN(attribute_name,(attribute_name::PARENTY)(attribute_name::CHILDY)))
				join += "_tree";
			return;
	}
}

static
void add_many2many_field_name(std::string& cxxstring,  const char* const attribute_name,  const char which_tbl){
	switch(which_tbl){
		case 'f':
			if (attribute_name == attribute_name::TAG)
				cxxstring += "tag";
			else
				cxxstring += "x";
			return;
		case 't':
			cxxstring += attribute_name;
			return;
		default:
			abort();
	}
}

static
bool is_valid_tbl2tbl_reference(const char which_tbl,  const arg::ArgType arg_token){
	const bool b = (
		   (CHAR_IN(which_tbl,('e')('f')('d')/*('D')*/) and ARGTYPE_IN(arg_token, (arg::tag)(arg::tag_tree)))
		or ((which_tbl=='f') and ARGTYPE_IN(arg_token, (arg::dir)(arg::dir_basename)))
		or ((which_tbl=='e') and (arg_token==arg::file))
	);
	
	log("is_valid_tbl2tbl_reference(", which_tbl, ",", tbl_arg_to_alias(arg_token), ") == ", (b?"TRUE":"FALSE"));
	
	return b;
}

static
bool has_intermediate_tbl_in_tbl2tbl_reference(const char which_tbl,  const arg::ArgType arg_token){
	return not (
		   ((which_tbl=='f') and (ARGTYPE_IN(arg_token, (arg::dir)(arg::dir_basename))))
		or ((which_tbl=='e') and (ARGTYPE_IN(arg_token, (arg::file))))
		//or ((which_tbl=='d') and (ARGTYPE_IN(arg_token, (arg::device)))) // arg::device is not implemented yet
	);
}

static
bool is_tree_tbl(const arg::ArgType arg){
	switch(arg){
		//case arg::dir: // NOTE: not defacto, as we parse the entire path's string
		case arg::tag_tree:
			return true;
		default:
			return false;
	}
}

static
void add_join_for_ersatz_attr(std::string& join,  const char* const attribute_name,  const char which_tbl,  const unsigned ersatz_count){
	join += "JOIN(SELECT ";
	join += tbl_full_name(which_tbl);
	join += " AS id, COUNT(*) AS x FROM ";
	join += get_tbl_name_of_join_in_ersatz_many_to_one(attribute_name, which_tbl);
	// TODO: Add security filter, e.g. join += " WHERE id NOT IN ..."
	join += " GROUP BY ";
	join += tbl_full_name(which_tbl);
	join += ")ersatz";
	join += std::to_string(ersatz_count);
	join += " ON ersatz";
	join += std::to_string(ersatz_count);
	join += ".id=X.id\n";
}

static
bool many2many_attr_has_its_own_tbl(const char* const attribute_name){
	return not (STR_IN(attribute_name,(attribute_name::DCT_HASH)(attribute_name::AUDIO_HASH)(attribute_name::MD5)(attribute_name::SHA256)(attribute_name::THUMBNAIL)));
}

static
const char* get_tbl_name_from_attr_name(const char* const attribute_name){
	if (STR_IN(attribute_name,(attribute_name::PARENT)(attribute_name::PARENTY)(attribute_name::CHILD)(attribute_name::CHILDY)))
		return "tag";
	return attribute_name;
}

#define X(_,n,data) or (attribute_name == attribute_name::data)
static
attribute_value_kind::AttributeValueKind get_attribute_value_kind(const char* const attribute_name){
	return (false BOOST_PP_SEQ_FOR_EACH(X, _, (DIR)(DURATION)(ID)(SIZE)(ERSATZ_SIZE)(STATUS)(T_ORIGIN)(T_RECORDED)(WIDTH)(HEIGHT)(LIKES)(DISLIKES)(FPS)(VIEWS)(LIKES))) ? attribute_value_kind::integer : attribute_value_kind::string;
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

struct Range {
	const char* min_start;
	const char* min_end;
	const char* max_start;
	const char* max_end;
	std::string_view min() const {
		return std::string_view(min_start,  compsky::utils::ptrdiff(min_end, min_start));
	}
	std::string_view max() const {
		return std::string_view(max_start,  compsky::utils::ptrdiff(max_end, max_start));
	}
};

template<typename Int>
void if_negatable_then_look_for_dash(const char*& qry){}

template<>
void if_negatable_then_look_for_dash<double>(const char*& qry){
	if(*qry == '-')
		++qry;
}

template<typename Int>
void loop_over_valid_char_for_conversion(const char*& qry){
	if (*qry == '0'){
		++qry;
		return;
	}
	while((*qry >= '0') and (*qry <= '9'))
		++qry;
}

template<>
void loop_over_valid_char_for_conversion<double>(const char*& qry){
	loop_over_valid_char_for_conversion<uint64_t>(qry);
	if (*qry != '.')
		return;
	++qry; // Skip decimal point
	while((*qry >= '0') and (*qry <= '9'))
		++qry;
}

template<typename Number>
successness::ReturnType get_range(const char*& qry,  Range& range){
	++qry; // Skip space
	range.min_start = qry;
	if_negatable_then_look_for_dash<Number>(qry);
	loop_over_valid_char_for_conversion<Number>(qry);
	range.min_end = qry;
	if ((range.min_end == range.min_start) or (range.min_end[-1] == '-'))
		return successness::invalid;
	
	if(*qry != '-')
		return successness::invalid;
	++qry;
	
	range.max_start = qry;
	if (*qry == '?'){
		constexpr static const char* const MAX_INT_AS_STR = "18446744073709551615"; // max of uint64_t
		range.max_start = MAX_INT_AS_STR;
		range.max_end = MAX_INT_AS_STR + std::char_traits<char>::length(MAX_INT_AS_STR);
		++qry;
	} else {
		if_negatable_then_look_for_dash<Number>(qry);
		loop_over_valid_char_for_conversion<Number>(qry);
		range.max_end = qry;
	}
	if ((range.max_end == range.max_start) or (range.max_end[-1] == '-'))
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
		if (unlikely(c == '\\')){
			if (*(++qry) == 0)
				return successness::invalid;
			continue;
		}
		if (c == '"')
			break;
		if (c == 0)
			return successness::invalid;
	}
	if (*(++qry) != ' ')
		return successness::invalid;
	const size_t sz = compsky::utils::ptrdiff(qry, qry_orig);
	if (unlikely(sz == 2))
		// "" is an illegal regexp according to MySQL/MariaDB
		return successness::invalid;
	result.append(qry_orig, sz);
	return successness::ok;
}

static
arg::ArgType process_arg(const char*& str){
	log("process_arg ", str);
	#include "auto-generated/qry-process_arg-tokens.hpp"
	return arg::invalid;
}

static
successness::ReturnType get_attribute_name(const char which_tbl,  const char*& str,  const char*& attribute_name,  attribute_kind::AttributeKind& attribute_kind){
	#include "auto-generated/qry-get_attribute_name-tokens.hpp"
	return successness::invalid;
}

static
successness::ReturnType process_name_list(std::string& where,  const char tbl_alias,  const char*& qry){
	log("process_name_list ", tbl_alias, qry);
	const size_t where_length_orig = where.size();
	where += " REGEXP BINARY "; // This is overwritten with " IN (   " if there is only one value in this list
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
						const size_t sz = compsky::utils::ptrdiff(qry, start);
						if (unlikely(sz == 0))
							return successness::invalid;
						where.append(start, sz);
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
				if (n_elements > 1){
					where.replace(where_length_orig, 15, " IN (   ");
					where += ")";
				}
				return successness::ok;
		}
	}
}

static
successness::ReturnType process_value_list(std::string& where,  const char*& qry){
	log("process_value_list ", qry);
	
	Range range;
	const auto rc = get_range<double>(qry, range);
	if (rc != successness::ok)
		return rc;
	
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
						where.append(start,  compsky::utils::ptrdiff(qry, start));
						where += " WHERE x>=";
						where += std::string_view(range.min());
						where += " AND x<=";
						where += std::string_view(range.max());
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
	log("process_order_by_var_name_list ", qry);
	
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
						const size_t sz = compsky::utils::ptrdiff(qry, start);
						
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
successness::ReturnType process_args(const std::string& connected_local_devices_str,  const char* const user_disallowed_X_tbl_filter_inner_pre,  const unsigned user_id,  const char*& select_fields,  std::string& join,  std::string& where,  std::string& order_by,  unsigned& limit,  unsigned& offset,  const char which_tbl,  const char* qry){
	log("process_args ", which_tbl, qry);
	constexpr size_t max_bracket_depth = 16; // Arbitrary limit
	constexpr static const char* const _operator_or  = "\nOR";
	constexpr static const char* const _operator_and = "\nAND";
	constexpr static const char* const _operator_none = "";
	const char* bracket_operator_at_depth[max_bracket_depth] = {_operator_none};
	unsigned bracket_depth = 0;
	int n_args_since_operator = 0;
	unsigned n_operators = 0;
	std::string join_for_auto_ordering = "";
	std::string auto_order_by = "";
	
	// Some arguments must only be encountered at most once
	unsigned n_calls__order_by = 0;
	unsigned n_calls__limit = 0;  // Doesn't reallly matter to the serverif multiple limits are specified
	unsigned n_calls__offset = 0; // Doesn't reallly matter to the serverif multiple offests are specified
	unsigned n_calls__backups = 0;
	unsigned n_calls__boxes = 0;
	unsigned n_calls__tags = 0;
	unsigned n_calls__eras = 0;
	
	unsigned ersatz_count = 0;
	
	const char* attribute_name;
	attribute_kind::AttributeKind attribute_kind;
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
				return (
					(bracket_depth == 0) and (
						(n_args_since_operator == 1) or (n_args_since_operator == 0  and  n_operators == 0)
					)
				) ? successness::ok : successness::invalid;
				// WARNING: This also disallows using no filters altogether.
				// TODO: Fix that.
			case arg::invalid:
				return successness::invalid;
			
			case arg::operator_or:
			case arg::operator_and:
				++n_operators;
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
			
			case arg::era:
			case arg::file:
			case arg::dir:
			case arg::dir_basename:
			case arg::tag:
			case arg::tag_tree: {
				if (not is_valid_tbl2tbl_reference(which_tbl, arg_token_base))
					return successness::invalid;
				where += bracket_operator_at_depth[bracket_depth];
				if (is_inverted)
					where += " NOT";
				where += " EXISTS(SELECT 1 FROM ";
				if (has_intermediate_tbl_in_tbl2tbl_reference(which_tbl, arg_token_base)){
					// e.g. for file to tag
					// 1st example branch
					where += tbl_full_name(which_tbl); // e.g. "file"
					where += "2";
					where += tbl_full_name(tbl_arg_to_alias(arg_token_base)); // e.g. "tag"
				} else {
					// 2nd example branch
					where += tbl_full_name(which_tbl); // e.g. "file"
				}
				where += " x JOIN ";
				if (is_tree_tbl(arg_token_base)){
					where += tbl_full_name(tbl_arg_to_alias(arg_token_base)); // e.g. "tag" or "dir"
					where += "2parent_tree z ON z.id=x.";
					where += tbl_full_name(tbl_arg_to_alias(arg_token_base)); // e.g. "tag"
					where += " JOIN ";
				}
				where += tbl_full_name(tbl_arg_to_alias(arg_token_base)); // e.g. "tag" or "dir"
				where += " y ON y.id=";
				if (is_tree_tbl(arg_token_base)){
					where += "z.parent";
				} else {
					where += "x.";
					where += tbl_full_name(tbl_arg_to_alias(arg_token_base)); // e.g. "tag" or "dir"
				}
				where += " WHERE x.";
				where += (has_intermediate_tbl_in_tbl2tbl_reference(which_tbl, arg_token_base)) ? tbl_full_name(which_tbl) : "id";
				where += "=X.id AND y.";
				where += (arg_token_base==arg::dir)?"full_path":"name";
				rc = process_name_list(where, tbl_arg_to_alias(arg_token_base), qry);
				if (rc != successness::ok)
					return rc;
				where += ")";
				++n_args_since_operator;
				break;
			}
			case arg::device_exists: {
				if ((which_tbl != 'f') and (which_tbl != 'd'))
					return successness::unimplemented;
				where += bracket_operator_at_depth[bracket_depth];
				where += " X.id ";
				if (is_inverted)
					where += "NOT ";
				if (which_tbl == 'f')
					where += "IN (SELECT f.id FROM file f JOIN dir d ON d.id=f.dir WHERE d.device IN(" + connected_local_devices_str + "))";
				else
					where += "IN (SELECT id FROM dir WHERE device IN(" + connected_local_devices_str + ")";
				++n_args_since_operator;
				break;
				break;
			}
			case arg::eras:
			case arg::boxes:
			case arg::tags:
			case arg::backups: {
				if (which_tbl != 'f')
					return successness::invalid;
				
				switch(arg_token_base){
					case arg::eras:
						if (++n_calls__eras == 2)
							return successness::invalid;
						break;
					case arg::backups:
						if (++n_calls__backups == 2)
							return successness::invalid;
						break;
					case arg::boxes:
						if (++n_calls__boxes == 2)
							return successness::invalid;
						break;
					case arg::tags:
						if (++n_calls__tags == 2)
							return successness::invalid;
						break;
					default:
						abort();
				}
				
				Range range;
				rc = get_range<uint64_t>(qry, range);
				if (rc != successness::ok)
					return rc;
				
				where += bracket_operator_at_depth[bracket_depth];
				where += " X.id ";
				if (is_inverted)
					where += "NOT ";
				where += "IN(SELECT file FROM ";
				where += get_tbl_for_file_assoc_count(arg_token_base);
				where += " GROUP BY file HAVING COUNT(*)>=";
				where += std::string_view(range.min());
				where += " AND COUNT(*)<=";
				where += std::string_view(range.max());
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
				
				switch(attribute_kind){
					case attribute_kind::many_to_many:
						join += "JOIN ";
						add_many2many_join_tbl_name(join, attribute_name, which_tbl);
						join += " f2_join ON f2_join.file=X.id\n";
						order_by = "f2_join.";
						order_by += attribute_field_name(attribute_name);
						break;
					case attribute_kind::unique:
					case attribute_kind::many_to_one:
						order_by = "X.";
						order_by += std::string(attribute_name);
						break;
					case attribute_kind::ersatz_many_to_one:
						++ersatz_count;
						order_by = "ersatz" + std::to_string(ersatz_count) + ".x ";
						add_join_for_ersatz_attr(join, attribute_name, which_tbl, ersatz_count);
						break;
					case attribute_kind::global_fn:
						order_by = attribute_name;
						break;
				}
				order_by += (arg_token==arg::order_by_asc)?" ASC":" DESC";
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
				
				Range range;
				rc = get_range<uint64_t>(qry, range);
				if (rc != successness::ok)
					return rc;
				
				const std::string old_where = where;
				if (attribute_kind == attribute_kind::unique)
					return successness::invalid;
				if (attribute_kind == attribute_kind::many_to_one){
					where = "X.";
					where += attribute_name;
					if (is_inverted)
						where += " NOT";
					where += " IN(SELECT X.";
					where += attribute_name;
					where += " AS x\nFROM ";
					where += tbl_full_name(which_tbl);
					where += " X\n";
					where += join;
					where += "WHERE ";
					where += (old_where.empty())?"TRUE":old_where;
					where += "\nAND ";
					where += user_disallowed_X_tbl_filter_inner_pre;
					where += "\nGROUP BY ";
					where += attribute_name;
					where += "\nHAVING COUNT(*)>=";
					where += std::string_view(range.min());
					where += " AND COUNT(*)<=";
					where += std::string_view(range.max());
					where += ")";
				} else if (attribute_kind == attribute_kind::ersatz_many_to_one){
					return successness::unimplemented;
				} else {
					where = "X.id ";
					if (is_inverted)
						where += "NOT ";
					where += "IN(SELECT file\n\tFROM file2";
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
					derived_tbl += tbl_full_name(which_tbl);
					derived_tbl += " X ON X.id=f2_.file\n\t\t";
					derived_tbl += join;
					derived_tbl += "\n\t\tWHERE ";
					derived_tbl += (old_where.empty())?"TRUE":old_where;
					derived_tbl += "\n\t\tAND ";
					derived_tbl += user_disallowed_X_tbl_filter_inner_pre;
					derived_tbl += "\n\t\tGROUP BY x";
					derived_tbl += "\nHAVING COUNT(*)>=";
					derived_tbl += std::string_view(range.min());
					derived_tbl += " AND COUNT(*)<=";
					derived_tbl += std::string_view(range.max());
					derived_tbl += "\n\t)";
					where += derived_tbl;
					where += "\n)";
					if (not is_inverted){
						join_for_auto_ordering = "JOIN file2" + std::string(attribute_name) + " f2_order ON f2_order.file=X.id\nJOIN " + derived_tbl + " f2_derived ON f2_order.x=f2_derived.x\n";
						auto_order_by = "f2_order.x DESC";
					}
				}
				if (not is_inverted)
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
					if (not has_name_attribute(which_tbl))
						return successness::invalid;
				} else {
					rc = get_attribute_name(which_tbl, qry, attribute_name, attribute_kind);
					if (rc != successness::ok)
						return rc;
					
					comparison_mode = *(++qry);
					if (not CHAR_IN(comparison_mode, ('>')('<')('=')('r')) or (*(++qry) != ' '))
						return successness::invalid;
				}
				
				where += bracket_operator_at_depth[bracket_depth];
				if (is_inverted)
					where += " NOT";
				
				const auto value_kind = get_attribute_value_kind(attribute_name);
				
				if (attribute_kind == attribute_kind::many_to_many){
					where += " X.id IN(SELECT ";
					where += many2many_attr_has_its_own_tbl(attribute_name) ?
								STR_IN(attribute_name, (attribute_name::CHILD)(attribute_name::CHILDY)) ?
									"parent"
									: "id"
								: tbl_full_name(which_tbl);
					where += " FROM ";
					add_many2many_join_tbl_name(where, attribute_name, which_tbl);
					where += " WHERE ";
					add_many2many_field_name(where, attribute_name, which_tbl);
					if (value_kind == attribute_value_kind::string){
						if (many2many_attr_has_its_own_tbl(attribute_name)){
							where += " IN(SELECT id FROM ";
							where += get_tbl_name_from_attr_name(attribute_name);
							where += " WHERE name";
						}
					}
				} else if ((attribute_kind == attribute_kind::many_to_one) or (attribute_kind == attribute_kind::unique)){
					where += " X.";
					where += attribute_name;
				} else /* if (attribute_kind == attribute_kind::ersatz_many_to_one) */ {
					++ersatz_count;
					
					where += " ersatz";
					where += std::to_string(ersatz_count);
					where += ".x ";
					
					add_join_for_ersatz_attr(join, attribute_name, which_tbl, ersatz_count);
				}
					
				switch(value_kind){
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
								where += " REGEXP BINARY ";
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
				
				if (attribute_kind == attribute_kind::many_to_many){
					if (value_kind == attribute_value_kind::string)
						if (many2many_attr_has_its_own_tbl(attribute_name))
							where += ")";
					where += ")";
				}
				
				++n_args_since_operator;
				break;
			}
			case arg::select_count:
				select_fields = selected_field::count;
				break;
			case arg::select_total_size:
				if (which_tbl != 'f')
					return successness::invalid;
				select_fields = selected_field::total_size;
				break;
			case arg::select_total_views:
				if (which_tbl != 'f')
					return successness::invalid;
				select_fields = selected_field::total_views;
				break;
			case arg::select_list:
				if (which_tbl == 'e')
					// TODO: Implement something for eras
					return successness::invalid;
				select_fields = (which_tbl == 'f') ? selected_field::list__file : selected_field::list;
				break;
			case arg::select_url_and_title__markdown:
				if (which_tbl != 'f')
					return successness::invalid;
				select_fields = selected_field::url_and_title__markdown;
				break;
			case arg::select_check_local_files:
				if (which_tbl != 'f')
					return successness::invalid;
				select_fields = selected_field::check_local_files;
				break;
			case arg::select_delete_local_files:
				if (which_tbl != 'f')
					return successness::invalid;
				select_fields = selected_field::delete_local_files;
				break;
			case arg::select_export_results:
				if (which_tbl != 'f')
					return successness::invalid;
				select_fields = selected_field::export_results;
				break;
			case arg::limit: {
				if (++n_calls__limit == 2)
					return successness::invalid;
				
				limit = s2n<unsigned>(qry);
				if (*qry != ' ')
					return successness::invalid;
				if (limit > MAX_LIMIT)
					limit = MAX_LIMIT;
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

selected_field::Type parse_into(char* itr,  const char* qry,  const std::string& connected_local_devices_str,  const unsigned user_id){
	// TODO: Look into filtering the filters with the user_id permission filters, to avoid brute-forcing.
	// Not sure this is a big issue.
	
	char which_tbl;
	std::string join = "";
	std::string where = "";
	std::string order_by = "";
	unsigned limit = 10;
	unsigned offset = 0;
	where.reserve(4096);
	switch(*qry){
		case 'e':
		case 'f':
		case 'd':
		case 't':
			which_tbl = *qry;
			break;
		default:
			return selected_field::INVALID;
	}
	switch(*(++qry)){
		case ' ': break;
		default: return selected_field::INVALID;
	}
	
	char* filter = itr;
	switch(which_tbl){
		case 'e':
			compsky::asciify::asciify(filter, NOT_DISALLOWED_ERA("X.id", "X.file", "0", "0", user_id));
			// WARNING: This does not yet conduct directory filtering
			break;
		case 'f':
			compsky::asciify::asciify(filter, NOT_DISALLOWED_FILE("X.id", "X.dir", "0", user_id));
			// WARNING: No device filtering is yet conducted
			break;
		case 'd':
			compsky::asciify::asciify(filter, NOT_DISALLOWED_DIR("X.id", "X.device", user_id));
			break;
		case 't':
			compsky::asciify::asciify(filter, NOT_DISALLOWED_TAG("X.id", user_id));
			break;
	}
	compsky::asciify::asciify(filter, '\0');
	const char* select_fields = selected_field::x_id;
	if (process_args(connected_local_devices_str, itr, user_id, select_fields, join, where, order_by, limit, offset, which_tbl, qry) != successness::ok){
		log("join == ", join.c_str());
		log("where == ", where.c_str());
		log("order_by == ", order_by.c_str());
		return selected_field::INVALID;
	}
	
	compsky::asciify::asciify(
		itr,
		"SELECT ",
			select_fields, "\n"
		"FROM ", tbl_full_name(which_tbl), " X\n",
		(which_tbl=='e') ? "JOIN file f ON f.id=X.file JOIN dir d ON d.id=f.dir " : "",
		(which_tbl=='f') ? "JOIN dir d ON d.id=X.dir " : "",
		join.c_str(),
		"WHERE ", where.c_str(), '\n',
		(where.empty()) ? "" : "AND "
	);
	
	switch(which_tbl){
		case 'e':
			compsky::asciify::asciify(itr, NOT_DISALLOWED_ERA("X.id", "X.file", "f.dir", "d.device", user_id));
			break;
		case 'f':
			compsky::asciify::asciify(itr, NOT_DISALLOWED_FILE("X.id", "X.dir", "d.device", user_id));
			break;
		case 'd':
			compsky::asciify::asciify(itr, NOT_DISALLOWED_DIR("X.id", "X.device", user_id));
			break;
		case 't':
			compsky::asciify::asciify(itr, NOT_DISALLOWED_TAG("X.id", user_id));
			break;
	}
	
	compsky::asciify::asciify(
		itr,
		"\n"
		"ORDER BY ", ((order_by.empty()) ? "NULL" : order_by.c_str()), "\n"
		"LIMIT ", limit, "\n"
		"OFFSET ", offset,
		'\0'
	);
	
	log("Query OK");
	
	return selected_field::get_enum(select_fields);
}

} // namespace sql_factory
