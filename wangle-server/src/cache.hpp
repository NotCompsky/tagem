/*
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
*/
#pragma once


namespace cached_stuff {
	// WARNING: This is only for functions whose results are guaranteed to be shorter than the max_buf_len.
	// TODO: Invalidate caches when necessary (after data is modified)
	constexpr static const size_t max_buf_len = 1  +  100 * (1 + 20 + 1 + 2*64 + 1 + 20 + 1 + 2*20 + 3 + 2*20 + 1 + 1 + 1)  +  1  +  1; // == 25803
	static char cache[n_cached * max_buf_len];
	enum CacheID {
		files_given_tag,
		files_given_dir,
		files_given_ids,
		tags_given_file,
		dir_info,
		file_info,
		n_fns
	};
	struct ID {
		unsigned int n_requests;
		unsigned int which_cached_fn;
		uint64_t user_id;
		size_t sz;
	};
	static ID cached_IDs[n_cached] = {}; // Initialise to zero
	
	int from_cache(const unsigned int which_cached_fn,  const uint64_t user_id){
		int i = 0;
		while (i < n_cached){
			ID& id = cached_IDs[i];
			++i;
			if ((id.which_cached_fn == which_cached_fn) and (id.user_id == user_id)){
				++id.n_requests;
				return i;
			}
		}
		return 0;
	}
	
	void add(const char* const buf,  const size_t sz,  const unsigned int which_cached_fn,  const uint64_t user_id,  const unsigned int n_requests = 1){
		unsigned int min_n_requests = UINT_MAX;
		unsigned int indx = 0; // In case all IDs have n_requesets at UINT_MAX - which is extremely unlikely
		for (unsigned int i = 0;  i < n_cached;  ++i){
			const ID& id = cached_IDs[i];
			if (id.n_requests >= min_n_requests)
				continue;
			indx = i;
			min_n_requests = id.n_requests;
		}
		
		memcpy(cache + (indx * max_buf_len),  buf,  sz);
		cached_IDs[indx].which_cached_fn = which_cached_fn;
		cached_IDs[indx].n_requests = n_requests;
		cached_IDs[indx].user_id = user_id;
		cached_IDs[indx].sz = sz;
	}
}
