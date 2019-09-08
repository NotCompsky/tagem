#include "file2.hpp"
#include "dropdown_dialog.hpp"
#include "name_dialog.hpp"
#include "utils.hpp"
#include <QRegExpValidator>
#include <map>


namespace file2 {

	QString prev_name = "";
	QStringList names;
	std::map<QString, uint64_t> name2max;
	QCompleter* names_completer;
	const QRegExpValidator names_validator(QRegExp("[0-9a-zA-Z$_]*"));
	const QStringList int_fields = {
		"BOOLEAN",
		"BOOLEAN UNSIGNED",
		"TINYINT",
		"TINYINT UNSIGNED",
		"SMALLINT",
		"SMALLINT UNSIGNED",
		"MEDIUMINT",
		"MEDIUMINT UNSIGNED",
		"INT",
		"INT UNSIGNED",
		"BIGINT"
		"BIGINT UNSIGNED"
	};
	int int_field_bits(const int indx){
		switch(indx){
			case 0:
			case 1:
				return 1;
			case 2:
			case 3:
				return 8;
			case 4:
			case 5:
				return 16;
			case 6:
			case 7:
				return 24;
			case 8:
			case 9:
				return 32;
			case 10:
			case 11:
				return 64;
		}
	}
	
	
	uint64_t calc_max_value(const uint8_t n_bits,  const bool is_unsigned){
		const uint64_t n = 2 * ((1 << (n_bits - 1)) - 1)  +  1;
		return n >> (is_unsigned ? 0 : 1);
		// equivalent to (1 << n_bits) - 1, but without the risk of overflow in the case n_bits==64
	}


	void initialise(){
		compsky::mysql::query_buffer(_mysql::obj, RES1, "SELECT n_bits, is_unsigned, name FROM file2");
		uint8_t n_bits;
		bool is_unsigned;
		const char* name;
		while(compsky::mysql::assign_next_row(RES1, &ROW1, &n_bits, &is_unsigned, &name)){
			names << name;
			name2max[name] = calc_max_value(n_bits, is_unsigned);
		}
		names_completer = new QCompleter(names);
	}


	uint64_t choose(QString& var_name){
		NameDialog* dialog = new NameDialog("Variable", prev_name);
		dialog->name_edit->setCompleter(names_completer);
		dialog->name_edit->setValidator(&names_validator);
		const auto rc = dialog->exec();
		var_name = dialog->name_edit->text();
		
		uint8_t max_value = 0;

		if (rc != QDialog::Accepted  ||  var_name.isEmpty())
			return max_value;

		if(names.contains(var_name)){
			max_value = name2max[var_name];
		} else {
			DropdownDialog* int_field_dialog = new DropdownDialog("Int Field", int_fields);
			const auto int_field_dialog_rc = int_field_dialog->exec();
			const int int_field_indx = int_field_dialog->combo_box->currentIndex();
			delete int_field_dialog;
			if (int_field_dialog_rc != QDialog::Accepted)
				return max_value;
			
			const uint8_t n_bits = int_field_bits(int_field_indx);
			
			const bool is_unsigned = int_field_indx % 2;
			
			max_value = calc_max_value(n_bits, is_unsigned);
			name2max[var_name] = max_value;
			
			compsky::mysql::exec(
				_mysql::obj,
				BUF,
				"CREATE TABLE file2", var_name, "("
					"file_id BIGINT UNSIGNED NOT NULL,"
					"x ", int_fields[int_field_indx], " NOT NULL,"
					"PRIMARY KEY (file_id)"
				")"
			);
			
			compsky::mysql::exec(
				_mysql::obj,
				BUF,
				"INSERT INTO file2 (n_bits, is_unsigned, name) VALUES (",
				n_bits,
				",",
				(is_unsigned) ? '1' : '0',
				",\"",
				var_name,
				"\")"
			);
			
			names << var_name;
			delete names_completer;
			names_completer = new QCompleter(names);
		}
		
		prev_name = var_name;
		
		return max_value;
	}

} // namespace file2
