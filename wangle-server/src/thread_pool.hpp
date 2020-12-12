#pragma once

#include <array>
#include <mutex>
#include <compsky/macros/likely.hpp>


#define MAX_N_HANDLERS (N_THREADS * 2) // NOTE: Arbitrary


template<typename Obj,  class Derived>
class ThreadPool {
  private:
	// Arrays are used because vectors cannot be made atomic
	std::array<bool, MAX_N_HANDLERS> is_in_use;
	std::array<Obj, MAX_N_HANDLERS> bufs;
	
  protected:
	void master_set(const Obj& obj,  const unsigned indx){
		// Force set an array element without thread-safety
		// Useful for when initialisation of a derived class involves the creation of such an element
		this->bufs[indx] = obj;
	}
	
  public:
	ThreadPool()
	: is_in_use{}
	, bufs{}
	{}
	
	~ThreadPool(){
		for (Obj& obj : this->bufs)
			static_cast<Derived*>(this)->kill_obj(obj);
	}
	
	Obj get(){
		static std::mutex mutex;
		
		auto i = 0;
		for (;  i < MAX_N_HANDLERS;  ++i)
			if (not this->is_in_use[i])
				break;
		if (unlikely(i == MAX_N_HANDLERS))
			// TODO: Do better
			exit(6060);
		this->is_in_use[i] = true;
		Obj buf = this->bufs[i];
		if (unlikely(buf == nullptr)){
			static_cast<Derived*>(this)->new_obj(buf);
			this->bufs[i] = buf;
		}
		return buf;
	}
	
	void free(const Obj& buf){
		auto i = 0;
		for (;  i < MAX_N_HANDLERS;  ++i)
			if (this->bufs[i] == buf)
				break;
		// 0 <= i < MAX_N_HANDLERS is guaranteed
		this->is_in_use[i] = false;
		// NOTE: This is safe, because it is guaranteed that it doesn't matter if this value is read in the mean time (if it is true it is ignored), and it is guaranteed that it is only written to by one thing at a time
	}
};
