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
