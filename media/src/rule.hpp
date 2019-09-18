#pragma once

#include <compsky/mysql/query.hpp>
#include <QRegularExpression>
#include <QString>


extern char BUF[];


namespace _mysql {
	extern MYSQL* obj;
}

namespace files_from_which {
	enum {
		stdin,
		directory,
		sql,
		bash,
		url,
		COUNT
	};
}

namespace start_from_which {
	enum {
		literal,
		regex,
		sql,
		COUNT
	};
}


class InlistFilterRules {
public:
	QRegularExpression filename_regexp; // Initialises to empty
	QString files_from;
	QString start_from;
	size_t file_sz_min;
	size_t file_sz_max;
	int w_min;
	int w_max;
	int h_min;
	int h_max;
	unsigned int files_from_which;
	unsigned int start_from_which;
	bool skip_tagged;
	bool skip_trans;
	bool skip_grey;
	
	InlistFilterRules()
	:	file_sz_min(0)
	,	file_sz_max(0)
	,	w_min(0)
	,	w_max(0)
	,	h_min(0)
	,	h_max(0)
	,	files_from_which(files_from_which::stdin)
	,	start_from_which(start_from_which::literal)
	,	skip_tagged(false)
	{}
	
	template<typename String>
	bool load(const String s){
		constexpr static const compsky::asciify::flag::Escape f_esc;
		
		MYSQL_RES* res;
		MYSQL_ROW  row;
		
		compsky::mysql::query(_mysql::obj,  res,  BUF,  "SELECT  filename_regexp, files_from, start_from, skip_tagged, skip_trans, skip_grey, files_from_which, start_from_which, file_sz_min, file_sz_max, w_max, w_min, h_max, h_min  FROM settings  WHERE name=\"", f_esc, '"', s, "\"");
		
		unsigned int count = 0;
		
		const char* _filename_regexp;
		const char* _files_from;
		const char* _start_from;
		bool _skip_tagged;
		bool _skip_trans;
		bool _skip_grey;
		unsigned int _files_from_which;
		unsigned int _start_from_which;
		size_t _file_sz_min;
		size_t _file_sz_max;
		int _w_max;
		int _w_min;
		int _h_max;
		int _h_min;
		while(compsky::mysql::assign_next_row(res, &row, &_filename_regexp, &_files_from, &_start_from, &_skip_tagged, &_skip_trans, &_skip_grey, &_files_from_which, &_start_from_which, &_file_sz_min, &_file_sz_max, &_w_max, &_w_min, &_h_max, &_h_min)){
			++count;
			
			this->filename_regexp.setPattern(_filename_regexp);
			this->files_from = _files_from;
			this->start_from = _start_from;

			this->skip_tagged = _skip_tagged;
			this->skip_trans  = _skip_trans;
			this->skip_grey   = _skip_grey;
			
			this->files_from_which = _files_from_which;
			this->start_from_which = _start_from_which;
			
			this->file_sz_min = _file_sz_min;
			this->file_sz_max = _file_sz_max;
			
			this->w_min = _w_min;
			this->w_max = _w_max;
			this->h_min = _h_min;
			this->h_max = _h_max;
		}
		
		return count;
		// WARNING: The int should only ever have the value 1 or 0
	};
};
