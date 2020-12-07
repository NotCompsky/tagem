#pragma once

// Prevent memory fragmentation by using a pool of fixed-size allocations

#include <array>
#include <mutex>

#define MAX_N_HANDLERS (N_THREADS * 2) // NOTE: Arbitrary


struct HandlerBufPool {
	// Arrays are used because vectors cannot be made atomic
	std::array<bool, MAX_N_HANDLERS> is_buf_used;
	std::array<char*, MAX_N_HANDLERS> bufs;
	
	HandlerBufPool()
	: is_buf_used{}
	, bufs{}
	{}
	
	char* get_buf(){
		static std::mutex mutex;
		
		auto i = 0;
		for (;  i < MAX_N_HANDLERS;  ++i)
			if (not this->is_buf_used[i])
				break;
		if (unlikely(i == MAX_N_HANDLERS))
			// TODO: Do better
			exit(6060);
		this->is_buf_used[i] = true;
		char* buf = this->bufs[i];
		if (unlikely(buf == nullptr)){
			buf = reinterpret_cast<char*>(malloc(HANDLER_BUF_SZ));
			this->bufs[i] = buf;
		}
		return buf;
	}
	void free_buf(const char* const buf){
		auto i = 0;
		for (;  i < MAX_N_HANDLERS;  ++i)
			if (this->bufs[i] == buf)
				break;
		// 0 <= i < MAX_N_HANDLERS is guaranteed
		this->is_buf_used[i] = false;
		// NOTE: This is safe, because it is guaranteed that it doesn't matter if this value is read in the mean time (if it is true it is ignored), and it is guaranteed that it is only written to by one thing at a time
	}
};
