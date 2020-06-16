#pragma once

#include "nullable_string_view.hpp"


typedef unsigned UserIDIntType;


namespace user_auth {


namespace SpecialUserID {
	enum SpecialUserID : UserIDIntType {
		invalid = 0,
		guest = 1
	};
	#define GUEST_ID_STR "1"
}


struct User {
	NullableStringView name;
	UserIDIntType id; // WARNING: User ID of 0 is reserved for INVALID, ID of 1 is reserved for GUEST.
	
	User(const char* const _name,  const UserIDIntType _id)
	: name(_name, strlen(_name))
	, id(_id)
	{}
};


std::vector<User> users;

UserIDIntType get_user_id(const NullableStringView username){
	if(username.data == nullptr)
		return SpecialUserID::guest;
	for(const User user : users){
		if(user.name == username)
			return user.id;
	}
	return SpecialUserID::invalid;
}


} // namespace user_auth
