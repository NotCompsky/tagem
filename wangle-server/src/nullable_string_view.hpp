#pragma once

struct NullableStringView {
	const char* data;
	size_t sz;
	
	constexpr
	NullableStringView()
	: data(nullptr)
	, sz(0)
	{}
};
