#pragma once


constexpr
void increment(const char** s){
	++(*s);
}
constexpr
void increment(const char*** s){
	++(**s);
}

constexpr
char char_value(const char* s){
	return *s;
}
constexpr
char char_value(const char** s){
	return **s;
}


template<typename Int,  typename StrOrStrPtr>
constexpr
Int a2n(StrOrStrPtr s){
	Int n = 0;
	while(char_value(s) >= '0'  &&  char_value(s) <= '9'){
		n *= 10;
		n += char_value(s) - '0';
		increment(&s);
	}
	return n;
}

static_assert(a2n<unsigned>("15") == 15);
static_assert(a2n<int>("5") == 5);
static_assert(a2n<uint64_t>("0") == 0);
static_assert(a2n<uint64_t>("!") == 0);


template<typename Float,  typename StrOrStrPtr>
constexpr
Float a2f(StrOrStrPtr s){
	uint64_t ns[2] = {0, 0};
	unsigned indx = 0;
	uint64_t divisor = 1;
	while(true){
		if(char_value(s) == '.'){
			if(indx)
				// If we have already encountered a decimal point
				return 0.0;
			++indx;
			divisor = 1; // Ignore the divisor_power set from the pre-decimal part
			increment(&s);
			continue;
		}
		if((char_value(s) < '0') || (char_value(s) > '9'))
			return (Float)ns[0] + (Float)ns[1] / (Float)divisor;
		ns[indx] *= 10;
		ns[indx] += char_value(s) - '0';
		divisor *= 10;
		increment(&s);
	}
}

static_assert(a2f<float>("0") == 0.0);
static_assert(a2f<float>("5") == 5.0);
static_assert(a2f<float>("0.0") == 0.0);
static_assert(a2f<float>("0.1") > 0.0);
static_assert(a2f<float>("0.1") < 0.2);
static_assert(a2f<float>("16.2") > 16.1);
static_assert(a2f<float>("16.2") < 16.3);
