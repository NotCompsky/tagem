#pragma once

#include "nullable_string_view.hpp"


typedef unsigned UserIDIntType;


namespace user_auth {


struct User {
	NullableStringView name;
	UserIDIntType id; // WARNING: User ID of 0 is reserved for guests.
	
	User(const char* const _name,  const UserIDIntType _id)
	: name(_name, strlen(_name))
	, id(_id)
	{}
};


std::vector<User> users;

UserIDIntType get_user_id(const NullableStringView username){
	for(const User user : users){
		if(user.name == username)
			return user.id;
	}
	return 0;
}


} // namespace user_auth
