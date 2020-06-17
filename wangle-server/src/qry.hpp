#pragma once


#define USER_DISALLOWED_FILES_INNER_PRE \
		"SELECT f2t.file " \
		"FROM user2blacklist_tag u2ht " \
		"JOIN tag2parent_tree t2pt ON t2pt.parent=u2ht.tag " \
		"JOIN file2tag f2t ON f2t.tag=t2pt.tag " \
		"WHERE u2ht.user="
#define USER_DISALLOWED_FILES(user_id) \
	"(" \
		USER_DISALLOWED_FILES_INNER_PRE, user_id, \
	")"
#define FILE_TBL_USER_PERMISSION_FILTER(user_id) \
	"AND f.id NOT IN" USER_DISALLOWED_FILES(user_id)
#define USER_DISALLOWED_TAGS_INNER_PRE \
		"SELECT t2pt.tag " \
		"FROM user2blacklist_tag u2ht " \
		"JOIN tag2parent_tree t2pt ON t2pt.parent=u2ht.tag " \
		"WHERE u2ht.user="
#define USER_DISALLOWED_TAGS(user_id) \
	"(" \
		USER_DISALLOWED_TAGS_INNER_PRE, user_id, \
	")"
#define USER_DISALLOWED_TAGS__COMPILE_TIME(user_id) \
	"(" \
		USER_DISALLOWED_TAGS_INNER_PRE user_id \
	")"
#define TAG_TBL_USER_PERMISSION_FILTER(user_id) \
	"AND t.id NOT IN" USER_DISALLOWED_TAGS(user_id)
#define USER_DISALLOWED_DEVICES_INNER_PRE \
		"SELECT device " \
		"FROM user2blacklist_device " \
		"WHERE user="
#define USER_DISALLOWED_DEVICES(user_id) \
	"(" \
		USER_DISALLOWED_DEVICES_INNER_PRE, user_id, \
	")"
#define USER_DISALLOWED_DEVICES__COMPILE_TIME(user_id) \
	"(" \
		USER_DISALLOWED_DEVICES_INNER_PRE user_id \
	")"
#define USER_DISALLOWED_DIRS_INNER_PRE \
		"SELECT dir " \
		"FROM user2blacklist_dir " \
		"WHERE user="
#define USER_DISALLOWED_DIRS(user_id) \
	"(" \
		USER_DISALLOWED_DIRS_INNER_PRE, user_id, \
	")"
#define USER_DISALLOWED_DIRS__COMPILE_TIME(user_id) \
	"(" \
		USER_DISALLOWED_DIRS_INNER_PRE user_id \
	")"
#define DIR_TBL_USER_PERMISSION_FILTER(user_id) \
	"AND d.id NOT IN" USER_DISALLOWED_DIRS(user_id)


namespace sql_factory{

namespace successness {
	enum ReturnType {
		ok,
		unimplemented,
		invalid,
		malicious
	};
}

successness::ReturnType parse_into(char* itr,  const char* qry,  const unsigned user_id);

#define QRY_USAGE \
	"USAGE\n" \
	"	[TABLE_ALIAS] [[FILTERS]] [[OPTIONS]]\n" \
	"\n" \
	"FILTERS\n" \
	"Each may be preceded by \"not\" (without the quotation marks) to invert the filter\n" \
	"Each must be separated by a logical operator - thus far the only implemented ones are \"and\" and \"or\" (use without the quotation marks)\n" \
	"	[TABLE_ALIAS] [[LIST OF NAMES]]\n" \
	"		Entries have relations to entries of one of these names in this table\n" \
	"		Each name is separated by a double quote (\")\n" \
	"		e.g. t \"English (Language)\" \"French (Language)\"\n" \
	"		TABLE_ALIAS must be one of:\n" \
	"			t (tag - matching against child tags too)\n" \
	"			T (tag)\n" \
	"			f (file)\n" \
	"			d (dir)\n" \
	"			D (dir - matching against only the basename)\n" \
	"			x (for file only - file appears in the named external database)\n" \
	"	value [MIN]-[MAX] [[LIST_OF_NAMES]]\n" \
	"		Entries have a variable named one of the listed names, which has a value in the specified range\n" \
	"		Each name is separated by a double quote (\")\n" \
	"	name [REGEX]\n" \
	"		Entries have a name that matches the regex\n" \
	"		The regex must be surrounded by double quotes (\")\n" \
	"	mime [REGEX]\n" \
	"		Filter a file's mimetype\n" \
	"		Only valid when filtering the file table\n" \
	"	same [ATTRIBUTE] [MODE] [COUNT]\n" \
	"		This filter is NOT to be preceded by a logical operator\n" \
	"		Only for the file table\n" \
	"		MODE must be one of \">\", \"<\", or \"=\", without the quotation marks\n" \
	"		COUNT must be a non-negative integer\n" \
	"		Filters results for files sharing an attribute, that attribute having [MODE COUNT] files associated with it.\n" \
	"		The order in which this filter is applied matters\n" \
	"			For many-to-one attributes (such as name, dir, mime, size, duration, md5, sha)\n" \
	"				All other filters declared before this filter apply WITH this filter\n" \
	"				All other filters declared after this filter apply AFTER this filter\n" \
	"			For many-to-many attributes (such as dct)\n" \
	"				It is a bit different\n" \
	"		EXAMPLES\n" \
	"			many-to-one\n" \
	"				f name \"foo\" same dir > 1\n" \
	"					Finds files matching the regexp \"foo\", so long as they share a directory with another file matching that regexp.\n" \
	"				f same dir > 1 name \"foo\"\n" \
	"					Finds files matching the regexp \"foo\", so long as there is another file in its directory\n" \
	"			many-to-many\n" \
	"				f name \"foo\" same dct > 1\n" \
	"					Finds files whose DCT hashes overlap with those of files with names matching the regexp \"foo\"\n" \
	"				f same dct > 1 name \"foo\"\n" \
	"					Finds files matching the regexp \"foo\", so long as there is another file with overlapping DCT hashes\n" \
	"				f same dct > 100\n" \
	"					List files who have a DCT hash that at least 100 other files also have\n" \
	"				f same dct < 2\n" \
	"					List files who have a unique DCT hash\n" \
	"		ATTRIBUTE must be one of\n" \
	"			many-to-one options:\n" \
	"				name (WARNING: Might take a VERY long time without other filters)\n" \
	"				dir\n" \
	"				mime\n" \
	"				size\n" \
	"				duration\n" \
	"				md5\n" \
	"				sha\n" \
	"			many-to-many options:\n" \
	"				dct (perceptual DCT hash)\n" \
	"				tag\n" \
	"\n" \
	"OPTIONS\n" \
	"	order [MODE] [ATTRIBUTE]\n" \
	"		MODE must be one of a (ascending) or d (descending)\n" \
	"		Order results by the ATTRIBUTE\n" \
	"		For many-to-many attributes, files with multiple matching attributes will appear multiple times\n" \
	"		ATTRIBUTE must be one of\n" \
	"			file table additional options: Any of the attributes listed under the \"same\" filter\n" \
	"			many-to-one options:\n" \
	"				name\n" \
	"			many-to-many options:\n" \
	"				tag\n" \
	"				dct\n" \
	"	order-by-value [MODE] [[LIST_OF_NAMES]]\n" \
	"		MODE must be one of a (ascending) or d (descending)\n" \
	"		Order results by the named variables, in that order.\n" \
	"		The order is left-to-right, the value being a chain of IFNULLs\n" \
	"	limit [INTEGER]\n" \
	"	offset [INTEGER]\n" \
	"\n" \
	"KNOWN BUGS\n" \
	"	At least one filter must be included\n" \
	"\n" \
	"EXAMPLES\n" \
	"	f t \"Music\" t \"English (Language)\" \"French (Language)\" \"German (Language)\" and value 90-100 \"Musicness\" \"Score\" and name \"[.]webm$\" limit 100\n" \
	"		List 100 WEBM files tagged Music and also tagged one of: English, French or German language, having either a musicness or score of between 90 and 100\n" \
	"	f name \"\\\\.mp4\" and d \"^/\" limit 100\n" \
	"		List 100 MP4 files on the server\n" \
	"	f d \"^https://www[.]youtube[.]com/\" order-by-value d \"Musicness\" \"Score\"\n" \
	"		List files from https://youtube.com/ in descending order of 'value', where the 'value' is either the file's \"Musicness\" or - if that is unavailable - the file's \"Score\"\n" \
	"	f d \"^http\" and not t \"Music\"\n" \
	"		List remote (over HTTP/HTTPS) files which are not tagged \"Music\"\n" \
	"	f not t \"!!ROOT TAG||\" and not name \"[.](json|xml)$\"\n" \
	"		List files which are not tagged with anything, ignoring JSON and XML files.\n" \
	"	f mime \"^image/\"\n" \
	"		List files with image mimetypes\n" \
	"	f ( t \"Music\" and t \"Video\" ) or ( t \"Music\" and mime \"^video/\" )\n" \
	"		List music videos\n" \
	"	f name \"[.](png|jpe?g)\" same dct > 60 limit 1000\n" \
	"		The command I used to locate all my neural style transfer renders\n" \
	"	f name \"[.](mp4)\" same dct > 10 limit 1000\n" \
	"		The command I used to locate videos by a Youtuber (using the same video intro in his videos)\n" \


} // namespace sql_factory
