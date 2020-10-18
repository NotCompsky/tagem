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
#include "qry.hpp"
#include <compsky/deasciify/a2n.hpp>
#include <cstdio>


int main(const int argc,  const char* const* const argv){
	if (argc != 3){
		fprintf(stderr, "USAGE: ./qry 'FILTER STRING HERE ' USER_ID\n");
		fprintf(stderr,
			#include "qry-help-text.txt"
		);
		return 1;
	}
	char buf[4096];
	const auto rc = sql_factory::parse_into(buf, argv[1], "0", a2n<unsigned>(argv[2]));
	if (rc != sql_factory::selected_field::INVALID)
		printf("%s\n", buf);
	return rc;
}
