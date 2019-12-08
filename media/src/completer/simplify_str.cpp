/*
 * CC BY-SA 4.0
 * 
 * Original code is copyright Akiva (https://codereview.stackexchange.com/users/119044/akiva) and was viewed from https://codereview.stackexchange.com/questions/116795/replacing-certain-characters-in-a-qstring
 */


#include <QRegularExpression>
#include <QString>


void simplify_str(const QString& in,  QString& out){
	out = in;
	// Performance: Eliminate characters you do not wish to have. 
	out.remove(QRegularExpression("[" + QRegularExpression::escape("`'!*,?|¡¿") + "]"));

	// Performance: Check for characters
	if (out.contains(QRegularExpression("[" + QRegularExpression::escape("$/:ÀÁÄÙÛÜàáäçèéêëïñóöùûüś") + "]"))){
		// Special Characters 
		out.replace(QRegularExpression("[" + QRegularExpression::escape(":/") + "]"), "-");
		out.replace(QRegularExpression("[$]"), "s");

		// Upper Case
		out.replace(QRegularExpression("[ÁÀ]"),   "A");
		out.replace(QRegularExpression("[Ä]"),    "Ae");
		out.replace(QRegularExpression("[ÜÛÙ]"),  "U");

		// Lower Case
		out.replace(QRegularExpression("[áà]"),   "a");
		out.replace(QRegularExpression("[ä]"),    "ae");
		out.replace(QRegularExpression("[ç]"),    "c");
		out.replace(QRegularExpression("[ëêéè]"), "e");
		out.replace(QRegularExpression("[ï]"),    "i");
		out.replace(QRegularExpression("[ñ]"),    "n");
		out.replace(QRegularExpression("[óö]"),   "o");
		out.replace(QRegularExpression("[ûù]"),   "u");
		out.replace(QRegularExpression("[ü]"),    "ue");
		out.replace(QRegularExpression("[ś]"),    "s");
	}
}
