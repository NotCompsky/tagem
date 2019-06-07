#ifndef __ASCIIFY_BASE__
#define __ASCIIFY_BASE__

#ifdef QT_GUI_LIB
  #include <QString>
#endif
#include "asciify_flags.hpp"

namespace compsky::asciify {

/* Base Cases */
void asciify();

void asciify(const char* s);

void asciify(const char c);

void asciify(flag::StrLen f,  const char* s,  const int sz);


#ifdef QT_GUI_LIB
void asciify(const QString& qs);
#endif


}

#endif
