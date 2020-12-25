#pragma once

#include "thread_pool.hpp"


class HandlerBufPool : public ThreadPool<char*, HandlerBufPool> {
  public:
	void new_obj(char*& buf) const {
		buf = reinterpret_cast<char*>(malloc(HANDLER_BUF_SZ));
	}
	void kill_obj(char* buf) const {}
};
