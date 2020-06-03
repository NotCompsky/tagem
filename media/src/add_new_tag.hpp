#ifndef TAGEM_MEDIA_ADD_NEW_TAG_HPP
#define TAGEM_MEDIA_ADD_NEW_TAG_HPP

#include "add_new_tag.hpp"
#include <inttypes.h> // for uint64_t
#include <QString>


uint64_t get_id_from_table(const char* const table_name,  const char* const entry_name);
void tag2parent_add(const uint64_t tagid,  const uint64_t parid);
void tag2parent_rm(const uint64_t tagid,  const uint64_t parid);
uint64_t add_new_tag(const QString& tagstr,  uint64_t tagid = 0);
uint64_t ask_for_tag(const QString& str = "");


#endif
