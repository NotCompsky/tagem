#pragma once

struct NullableStringView {
	const char* data;
	size_t sz;
	
	constexpr
	NullableStringView()
	: data(nullptr)
	, sz(0)
	{}
	
	constexpr
	NullableStringView(const char* const _str,  const size_t sz)
	: data(_str)
	, sz(sz)
	{}
	
	bool operator==(const NullableStringView& other) const {
		if(other.sz != this->sz)
			return false;
		size_t i = 0;
		while(i < this->sz){
			if(this->data[i] != other.data[i])
				return false;
			++i;
		}
		return true;
	}
	
	bool operator<(const NullableStringView other) const {
		if(other.sz != this->sz)
			return (this->sz < other.sz);
		size_t i = 0;
		while(i < this->sz){
			if(this->data[i] != other.data[i])
				return (this->data[i] < other.data[i]);
			++i;
		}
		return false;
	}
};
