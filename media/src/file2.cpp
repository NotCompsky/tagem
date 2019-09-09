#include "file2.hpp"
#include "dropdown_dialog.hpp"
#include "name_dialog.hpp"
#include "utils.hpp"
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QRegExpValidator>
#include <map>


namespace file2 {

	QString prev_name = "";
	QStringList names;
	std::map<QString, MinMax> name2max;
	QCompleter* names_completer;
	const QRegExpValidator names_validator(QRegExp("[0-9a-zA-Z$_]*"));
	constexpr static const uint64_t int_field_diffs[] = {
		9223372036854775807,
		4294967295,
		16777215,
		65535,
		255,
		1
	};
	static const QStringList int_fields = {
		"BIGINT UNSIGNED",
		"BIGINT",
		"INT UNSIGNED",
		"INT",
		"MEDIUMINT UNSIGNED",
		"MEDIUMINT",
		"SMALLINT UNSIGNED",
		"SMALLINT",
		"TINYINT UNSIGNED",
		"TINYINT",
		"BOOLEAN"
	};


	void initialise(){
		compsky::mysql::query_buffer(_mysql::obj, RES1, "SELECT min, max, name FROM file2");
		int64_t min;
		int64_t max;
		const char* name;
		while(compsky::mysql::assign_next_row(RES1, &ROW1, &min, &max, &name)){
			names << name;
			name2max.try_emplace(name, min, max);
		}
		names_completer = new QCompleter(names);
	}

	bool is_valid_int_field_indx(const int indx,  const int64_t min,  const int64_t max){
		if (indx % 2  ==  0){
			// unsigned type
			if (min < 0)
				return false;
			if (max > int_field_diffs[indx / 2]){
				return false;
			}
			return true;
		}
		const uint64_t int_field_diff = int_field_diffs[indx / 2] / 2;
		if (max > int_field_diff  &&  -min > int_field_diff)
			return false;
		return true;
	}

	const MinMax zeros(0, 0);
	MinMax choose(QString& var_name){
		NameDialog* dialog = new NameDialog("Variable", prev_name);
		dialog->name_edit->setCompleter(names_completer);
		dialog->name_edit->setValidator(&names_validator);
		const auto rc = dialog->exec();
		var_name = dialog->name_edit->text();

		MinMax minmax = zeros;
		
		if (rc != QDialog::Accepted  ||  var_name.isEmpty())
			return zeros;

		prev_name = var_name;
		
		if(names.contains(var_name)){
			return name2max.at(var_name);
		}
		
		{
			bool ok;
			const int64_t min = QInputDialog::getInt(nullptr, "Variable", "Minimum", 0, INT_MIN, INT_MAX, 1, &ok);
			if (!ok)
				return zeros;
			const int64_t max = QInputDialog::getInt(nullptr, "Variable", "Maximum", 0, INT_MIN, INT_MAX, 1, &ok);
			if (!ok)
				return zeros;
			
			if(min >= max){
				QMessageBox::warning(nullptr, "Invalid Value", "Minimum >= Maximum");
				return zeros;
			}
			
			name2max.try_emplace(var_name, min, max);
			
			const bool is_unsigned = (min >= 0);
			
			const uint64_t diff = max - min;
			
			
			/* Suggest the minimum size int type that fulfils min-max requirement */
			unsigned int int_field_indx;
			for (int_field_indx = 0;  int_field_indx < std::size(int_field_diffs);  ++int_field_indx)
				if (diff > int_field_diffs[int_field_indx])
					break;
			
			if (int_field_indx == 0){
				// I don't think it is possible to trigger this - Qt5 only accepts int returns, and I doubt there are systems with sizeof(int) > sizeof(uint64_t)
				QMessageBox::warning(nullptr, "Invalid Value", "Possible range (max - min) exceeds uint64_t maximum (2^63 - 1)");
				return zeros;
			}
			
			--int_field_indx;
			
			int_field_indx *= 2;
			
			if (min < 0){
				// Possibility of using signed values for convenience
				++int_field_indx;
				if (int_field_indx >= int_fields.size()){
					QMessageBox::warning(nullptr, "Invalid Value", "Possible range (max - min) fits in boolean range, but the min value is sub-zero. It would require an offset, but that has not been coded in.");
					return zeros;
				}
			} else if (max > int_field_diffs[int_field_indx / 2]){
				if (int_field_indx == 0){
					QMessageBox::warning(nullptr, "Invalid Value", "Possible range (max - min) fits in uint64_t range, but the max's value exceeds it. It would require an offset, but that has not been coded in.");
					return zeros;
				}
				int_field_indx -= 2;
			}
			qDebug() << "Preselecting int_fields[" << int_field_indx << "] == " << int_fields[int_field_indx];
			
			
			while(true){
				DropdownDialog* int_field_dialog = new DropdownDialog("Int Field", int_fields);
				int_field_dialog->combo_box->setCurrentIndex(int_field_indx);
				const auto int_field_dialog_rc = int_field_dialog->exec();
				const int int_field_indx__chosen = int_field_dialog->combo_box->currentIndex();
				delete int_field_dialog;
				if (int_field_dialog_rc != QDialog::Accepted)
					return zeros;
				if (!is_valid_int_field_indx(int_field_indx__chosen, min, max))
					QMessageBox::warning(nullptr, "Too Small Int Type", "Int type cannot fit min-max values set");
				else {
					int_field_indx = int_field_indx__chosen;
					break;
				}
			}
			
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
				"INSERT INTO file2 (min, max, name) VALUES (",
				min,
				",",
				max,
				",\"",
				var_name,
				"\")"
			);
			
			names << var_name;
			delete names_completer;
			names_completer = new QCompleter(names);
			
			return name2max.at(var_name);;
		}
	}

} // namespace file2
