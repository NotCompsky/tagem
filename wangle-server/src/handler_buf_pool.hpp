#pragma once

#include "thread_pool.hpp"


class HandlerBufPool : public ThreadPool<char*, HandlerBufPool> {
  public:
	char* new_obj() const {
		return reinterpret_cast<char*>(malloc(HANDLER_BUF_SZ));
	}
};
