#include "qry.hpp"
#include <cstdio>


int main(const int argc,  const char* const* const argv){
	if (argc != 2){
		fprintf(stderr, QRY_USAGE);
		return 1;
	}
	char buf[4096];
	const auto rc = sql_factory::parse_into(buf, argv[1]);
	if (rc == sql_factory::successness::ok)
		printf("%s\n", buf);
	return rc;
}
