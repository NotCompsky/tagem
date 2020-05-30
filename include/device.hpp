#pragma once

#include "protocol.hpp"
#ifdef QT_VERSION
# include "dropdown_dialog.hpp"
# include <QInputDialog>
#else
# include "cli.hpp"
#endif
#include <compsky/mysql/query.hpp>


namespace _mysql {
	extern MYSQL* obj;
}

extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;
extern char BUF[];


inline
uint64_t get_device_id_from_file_path(const char* const file_path,  unsigned& protocol);


inline
unsigned guess_protocol(const char* url){
	if (*url == '/')
		return protocol::local_filesystem;
	
	unsigned protocol_guess = protocol::NONE;
	get_device_id_from_file_path(url, protocol_guess);
	if (protocol_guess != protocol::NONE)
		return protocol_guess;
	
	// Now follows an *extremely* crude method for guessing the protocol
	
	switch(*url){
		case 'h':
			switch(*(++url)){
				case 't':
					switch(*(++url)){
						case 't':
							switch(*(++url)){
								case 'p':
									switch(*(++url)){
										case 's':
											protocol_guess = protocol::https;
										case '/':
											protocol_guess = protocol::http;
									}
							}
					}
			}
	}
	if (protocol_guess == protocol::NONE)
		return protocol_guess;
	
	while(*url != 0)
		++url;
	unsigned n_periods = 0;
	for (auto i = 0;  i < 5;  ++i){
		switch(*(--url)){
			// Match likely media extensions
			case '.':
				++n_periods;
			case 'A' ... 'Z':
			case 'a' ... 'z':
			case '0' ... '9':
				continue;
			default:
				break;
		}
	}
	if (n_periods == 0){
#ifdef QT_VERSION
		DropdownDialog* const dialog = new DropdownDialog(QString("Protocol of ") + QString(url), {protocol::strings[protocol_guess], protocol::strings[protocol::youtube_dl]}, nullptr);
		dialog->exec();
		protocol_guess = (dialog->combo_box->currentIndex()) ? protocol::youtube_dl : protocol_guess;
		delete dialog;
#else
		constexpr static const cli::flag::ArgSeparator f;
		protocol_guess = (cli::get_which("Protocol of %s: ", url, f, protocol::strings[protocol_guess], protocol::strings[protocol::youtube_dl])) ? protocol_guess : protocol::youtube_dl;
#endif
	}
	
	return protocol_guess;
}


inline
unsigned guess_protocol(const char* const url [[unused]],  const unsigned protocol_id){
	if (protocol_id)
		return protocol_id;
	return guess_protocol(url);
}


/*
inline
bool occupy_same_device(const char* const a,  const char* const b){
	constexpr static const compsky::asciify::flag::Escape f_esc;
	compsky::mysql::query(_mysql::obj, RES1, BUF, "SELECT id FROM device WHERE name=LEFT(\"", f_esc, '"', a, "\", LENGTH(name)) AND name=LEFT(\"", f_esc, '"', b, "\"");
	unsigned really_a_boolean = 0;
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &really_a_boolean));
	return (really_a_boolean);
}
Does not work if we allow / and /media/external-hdd-1 to be different devices.
*/


inline
uint64_t get_device_id(const char* const device_path){
	constexpr static const compsky::asciify::flag::Escape f_esc;
	compsky::mysql::query(_mysql::obj, RES1, BUF, "SELECT id FROM device WHERE name=\"", f_esc, '"', device_path, "\" ORDER BY LENGTH(name) DESC LIMIT 1"); // Should allow e.g. /media/external-hdd-1 to be separate device from / - but files under /media/external-hdd-2 will be assumed to belong to the same device as / unless /media/external-hdd-2 is added to the device table.
	uint64_t device_id = 0;
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &device_id));
	return device_id;
}


inline
uint64_t get_device_id_from_file_path(const char* const file_path){
	constexpr static const compsky::asciify::flag::Escape f_esc;
	compsky::mysql::query(_mysql::obj, RES1, BUF, "SELECT id FROM device WHERE name=LEFT(\"", f_esc, '"', file_path, "\", LENGTH(name)) ORDER BY LENGTH(name) DESC LIMIT 1");
	uint64_t device_id = 0;
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &device_id));
	return device_id;
}


inline
uint64_t get_device_id_from_file_path(const char* const file_path,  unsigned& protocol){
	constexpr static const compsky::asciify::flag::Escape f_esc;
	compsky::mysql::query(_mysql::obj, RES1, BUF, "SELECT id, protocol FROM device WHERE name=LEFT(\"", f_esc, '"', file_path, "\", LENGTH(name)) ORDER BY LENGTH(name) DESC LIMIT 1");
	uint64_t device_id = 0;
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &device_id, &protocol));
	return device_id;
}


template<typename... Args>
uint64_t get_device_id__insert_if_not_exist(const char* const file_path,  Args... args){
	const uint64_t device_id = get_device_id_from_file_path(file_path, args...);
	if (device_id)
		return device_id;
	constexpr const char* const help_txt = "This filepath is from a new 'device'. Please input the device prefix (which only involves deleting characters from the end of the string). For instance, for YouTube videos, the 'device' would be https://www.youtube.com/watch?v=";
#ifdef QT_VERSION
	bool ok;
	const QString prefix = QInputDialog::getText(
		nullptr,
		"New device prefix",
		help_txt,
		QLineEdit::Normal,
		file_path,
		&ok
	);
#else
	const char* const prefix = cli::get_trim(file_path, "New device prefix.\n%s\n", help_txt);
#endif
	const unsigned protocol = guess_protocol(file_path, args...);
	compsky::mysql::exec(_mysql::obj, BUF, "INSERT INTO device (name, protocol) VALUES (\"", prefix, "\", ", protocol, ")");
	return get_device_id(file_path);
}
