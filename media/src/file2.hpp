#include <QString>
#include <QStringList>
#include <QCompleter>


namespace file2 {

	extern QString prev_name;
	extern QStringList names;
	extern QCompleter* names_completer;

	void initialise();
	uint64_t choose(QString& var_name);

} // namespace file2
