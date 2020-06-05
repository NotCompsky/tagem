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
	"	value [MIN]-[MAX] [[LIST_OF_NAMES]]\n" \
	"		Entries have a variable named one of the listed names, which has a value in the specified range\n" \
	"		Each name is separated by a double quote (\")\n" \
	"	name [REGEX]\n" \
	"		Entries have a name that matches the regex\n" \
	"		The regex must be surrounded by double quotes (\")\n" \
	"OPTIONS\n" \
	"	order [STRING]\n" \
	"		Order results by the STRING\n" \
	"		The string must be surrounded by double quotes (\")\n" \
	"	limit [INTEGER]\n" \
	"\n" \
	"EXAMPLES\n" \
	"	f t \"Music\" t \"English (Language)\" \"French (Language)\" \"German (Language)\" value 90-100 \"Musicness\" \"Score\" name \"[.]webm$\" limit 100\n" \
	"		List 100 WEBM files tagged Music and also tagged one of: English, French or German language, having either a musicness or score of between 90 and 100\n" \
	"	f name \"\\\\.mp4\" d \"^/\" limit 100\n" \
	"		List 100 MP4 files on the server\n" \


} // namespace sql_factory
