#pragma once

#include <QString>
#include <QStringList>
#include <QCompleter>


namespace file2 {

	namespace conversion {
		enum {
			integer,
			string,
			datetime,
			n
		};
	}
	
	extern QString prev_name;
	extern QStringList names;
	extern QCompleter* names_completer;

	struct MinMaxCnv {
		// Qt5 only allows integers in the range of int or unsigned int
		// int64_t is sufficient to cover both, assuming...
		static_assert(sizeof(int64_t) >= 2 * sizeof(int));
		
		const int64_t min;
		const int64_t max;
		const int unsigned cnv;
		
		MinMaxCnv(const int64_t _min,  const int64_t _max,  const int unsigned _cnv)
		: min(_min)
		, max(_max)
		, cnv(_cnv)
		{}
	};

	void initialise();
	MinMaxCnv choose(QString& var_name);

} // namespace file2
