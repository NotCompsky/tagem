/*
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
*/
#pragma once

#define USE_VECTOR
#include <compsky/asciify/asciify.hpp>


namespace _f {
	using namespace compsky::asciify::flag;
	constexpr static const Escape esc;
	constexpr static const esc::DoubleQuote esc_dblqt;
	constexpr static const AlphaNumeric alphanum;
	constexpr static const StrLen strlen;
	constexpr static const JSONEscape json_esc;
	constexpr static const Repeat repeat;
	constexpr static const Zip<3> zip3;
	constexpr static const NElements n_elements;
	constexpr static const Hex hex;
	constexpr static const grammatical_case::Lower lower_case;
	constexpr static const grammatical_case::Upper upper_case;
	constexpr static const esc::SpacesAndNonAscii esc_spaces_and_non_ascii;
	constexpr static const esc::URI_until_space::Unescape unescape_URI_until_space;
	constexpr static const UntilNullOr until;
}
