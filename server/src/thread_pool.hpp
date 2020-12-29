#pragma once

#include <array>
#include <mutex>
#include <compsky/macros/likely.hpp>


#define MAX_N_HANDLERS (N_THREADS * 2) // NOTE: Arbitrary


template<typename Obj,  class Derived,  size_t max_objs = MAX_N_HANDLERS>
class ThreadPool {
  private:
	// Arrays are used because vectors cannot be made atomic
	std::array<bool, max_objs> is_in_use;
	std::array<Obj, max_objs> bufs;
	
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
		
		size_t i = 0;
		for (;  i < max_objs;  ++i)
			if (not this->is_in_use[i])
				break;
		if (unlikely(i == max_objs))
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
		size_t i = 0;
		for (;  i < max_objs;  ++i)
			if (this->bufs[i] == buf)
				break;
		// 0 <= i < max_objs is guaranteed
		this->is_in_use[i] = false;
		// NOTE: This is safe, because it is guaranteed that it doesn't matter if this value is read in the mean time (if it is true it is ignored), and it is guaranteed that it is only written to by one thing at a time
	}
};
