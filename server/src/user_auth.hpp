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

#include "nullable_string_view.hpp"
#include <vector>


typedef unsigned UserIDIntType;


namespace user_auth {


namespace SpecialUserID {
	enum SpecialUserID : UserIDIntType {
		invalid = 0,
		guest = 1,
		admin = 2
	};
	#define GUEST_ID_STR "1"
	#define ADMIN_ID_STR "2"
}


struct User {
	NullableStringView name;
	UserIDIntType id; // WARNING: User ID of 0 is reserved for INVALID, ID of 1 is reserved for GUEST.
	
	char* allowed_file2_vars_csv;
	std::vector<const char*> allowed_file2_vars;
	
	User(const char* const _name,  const UserIDIntType _id,  char* _allowed_file2_vars)
	: name(_name, strlen(_name))
	, id(_id)
	, allowed_file2_vars_csv(nullptr)
	{
		if(*_allowed_file2_vars == 0)
			return;
		this->allowed_file2_vars_csv = reinterpret_cast<char*>(malloc(strlen(_allowed_file2_vars) + 1));
		if (unlikely(allowed_file2_vars_csv == nullptr))
			abort();
		memcpy(this->allowed_file2_vars_csv,  _allowed_file2_vars,  strlen(_allowed_file2_vars) + 1);
		while(true){
			const char* const file2_var_start = _allowed_file2_vars;
			// NOTE: First character cannot be a comma
			while((*_allowed_file2_vars != ',') and (*_allowed_file2_vars != 0))
				++_allowed_file2_vars;
			this->allowed_file2_vars.push_back(file2_var_start);
			if(*_allowed_file2_vars == 0)
				break;
			*_allowed_file2_vars = 0;
			++_allowed_file2_vars;
		}
	}
	
	~User(){
		if(this->allowed_file2_vars_csv != nullptr)
			free(this->allowed_file2_vars_csv);
	}
};


std::vector<User> users;

User* get_user(NullableStringView username){
	if(username.data == nullptr){
		username.data = "GUEST";
		username.sz   = 5;
	}
	for(User& user : users){
		if(user.name == username)
			return &user;
	}
	return nullptr;
}

UserIDIntType get_user_id(const NullableStringView username){
	if(username.data == nullptr)
		return SpecialUserID::guest;
	for(const User& user : users){
		if(user.name == username)
			return user.id;
	}
	return SpecialUserID::invalid;
}


} // namespace user_auth
