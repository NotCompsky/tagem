#include "qry.hpp"
#include "convert_str.hpp"
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
	const auto rc = sql_factory::parse_into(buf, argv[1], a2n<unsigned>(argv[2]));
	if (rc == sql_factory::successness::ok)
		printf("%s\n", buf);
	return rc;
}
