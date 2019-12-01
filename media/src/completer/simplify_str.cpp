/*
 * CC BY-SA 4.0
 * 
 * Original code is copyright Akiva (https://codereview.stackexchange.com/users/119044/akiva) and was viewed from https://codereview.stackexchange.com/questions/116795/replacing-certain-characters-in-a-qstring
 */


#include <QRegularExpression>
#include <QString>


QString simplify_str(const QString& str){
	QString s = str;
	// Performance: Eliminate characters you do not wish to have. 
	s.remove(QRegularExpression("[" + QRegularExpression::escape("`'!*,?|¡¿") + "]"));

	// Performance: Check for characters
	if (s.contains(QRegularExpression("[" + QRegularExpression::escape("$/:ÀÁÄÙÛÜàáäçèéêëïñóöùûüś") + "]"))){
		// Special Characters 
		s.replace(QRegularExpression("[" + QRegularExpression::escape(":/") + "]"), "-");
		s.replace(QRegularExpression("[$]"), "s");

		// Upper Case
		s.replace(QRegularExpression("[ÁÀ]"),   "A");
		s.replace(QRegularExpression("[Ä]"),    "Ae");
		s.replace(QRegularExpression("[ÜÛÙ]"),  "U");

		// Lower Case
		s.replace(QRegularExpression("[áà]"),   "a");
		s.replace(QRegularExpression("[ä]"),    "ae");
		s.replace(QRegularExpression("[ç]"),    "c");
		s.replace(QRegularExpression("[ëêéè]"), "e");
		s.replace(QRegularExpression("[ï]"),    "i");
		s.replace(QRegularExpression("[ñ]"),    "n");
		s.replace(QRegularExpression("[óö]"),   "o");
		s.replace(QRegularExpression("[ûù]"),   "u");
		s.replace(QRegularExpression("[ü]"),    "ue");
		s.replace(QRegularExpression("[ś]"),    "s");
	}
	
	return s;
}
