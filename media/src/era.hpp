#pragma once

#include <inttypes.h> // for uint64_t


extern char era2s_tblname[128];


struct Era {
	uint64_t id;
	uint64_t start;
	uint64_t end;
	uint64_t start_method;
	uint64_t end_method;
	unsigned int methods_yet_triggered;
	
	Era(uint64_t _id,  uint64_t _start,  uint64_t _end)
	: id(_id)
	, start(_start)
	, end(_end)
	, start_method(0)
	, end_method(0)
	, methods_yet_triggered(2)
	{}
	
	Era(uint64_t _id,  uint64_t _start,  uint64_t _end,  uint64_t _start_method,  uint64_t _end_method)
	: id(_id)
	, start(_start)
	, end(_end)
	, start_method(_start_method)
	, end_method(_end_method)
	, methods_yet_triggered(2)
	{}
};
