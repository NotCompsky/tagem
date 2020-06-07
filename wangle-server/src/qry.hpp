#pragma once


namespace sql_factory{

namespace successness {
	enum ReturnType {
		ok,
		unimplemented,
		invalid,
		malicious
	};
}

successness::ReturnType parse_into(char* itr,  const char* qry);

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
	"			x (external database)\n" \
	"	value [MIN]-[MAX] [[LIST_OF_NAMES]]\n" \
	"		Entries have a variable named one of the listed names, which has a value in the specified range\n" \
	"		Each name is separated by a double quote (\")\n" \
	"	name [REGEX]\n" \
	"		Entries have a name that matches the regex\n" \
	"		The regex must be surrounded by double quotes (\")\n" \
	"	mime [REGEX]\n" \
	"		Filter a file's mimetype\n" \
	"		Only valid when filtering the file table\n" \
	"\n" \
	"OPTIONS\n" \
	"	order [STRING]\n" \
	"		Order results by the STRING\n" \
	"		The string must be surrounded by double quotes (\")\n" \
	"	order-by-value [MODE] [[LIST_OF_NAMES]]\n" \
	"		MODE must be one of a (ascending) or d (descending)\n" \
	"		Order results by the named variables, in that order.\n" \
	"		The order is left-to-right, the value being a chain of IFNULLs\n" \
	"	limit [INTEGER]\n" \
	"	offset [INTEGER]\n" \
	"\n" \
	"KNOWN BUGS\n" \
	"	At least one filter must be included\n" \
	"EXAMPLES\n" \
	"	f t \"Music\" t \"English (Language)\" \"French (Language)\" \"German (Language)\" and value 90-100 \"Musicness\" \"Score\" and name \"[.]webm$\" limit 100\n" \
	"		List 100 WEBM files tagged Music and also tagged one of: English, French or German language, having either a musicness or score of between 90 and 100\n" \
	"	f name \"\\\\.mp4\" and d \"^/\" limit 100\n" \
	"		List 100 MP4 files on the server\n" \
	"	f d \"^https://www[.]youtube[.]com/\" order-by-value DESC \"Musicness\" \"Score\"\n" \
	"		List files from https://youtube.com/ in descending order of 'value', where the 'value' is either the file's \"Musicness\" or - if that is unavailable - the file's \"Score\"\n" \
	"	f d \"^http\" and not t \"Music\"\n" \
	"		List remote (over HTTP/HTTPS) files which are not tagged \"Music\"\n" \
	"	f not t \"!!ROOT TAG||\" and not name \"[.](json|xml)$\"\n" \
	"		List files which are not tagged with anything, ignoring JSON and XML files.\n" \
	"	f mime \"^image/\"\n" \
	"		List files with image mimetypes\n" \
	"	f ( t \"Music\" and t \"Video\" ) or ( t \"Music\" and mime \"^video/\" )\n" \
	"		List music videos\n" \


} // namespace sql_factory
