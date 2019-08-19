/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef TAGEM_MEDIA_UNLINK_TAG_BTN_HPP
#define TAGEM_MEDIA_UNLINK_TAG_BTN_HPP


#include <QMouseEvent>
#include <QPushButton>


class UnlinkTagBtn : public QPushButton{
  private:
	void mousePressEvent(QMouseEvent* e); // SLOT
	const char* const delete_from_where;
	const uint64_t primary_id;
	const char* const and_tag_id_eql;
  public:
	void exec(); // SLOT
	const uint64_t tag_id;
	explicit UnlinkTagBtn(const char* const _delete_from_where,  const uint64_t _primary_id,  const char* const and_tag_id_eql,  const uint64_t _tag_id,  QWidget* parent);
};


#endif
