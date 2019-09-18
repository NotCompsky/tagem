#include "rule.hpp"
#include <compsky/mysql/query.hpp>


constexpr static const size_t MEDIA_FP_SZ = 4096;
char FP[MEDIA_FP_SZ];

MYSQL_RES* res;
MYSQL_ROW  row;
QProcess files_from_bash;


bool iteration(const InlistFilterRules& r){
	const char* _fp;
	switch(r.files_from_which){
		case files_from_which::sql:
			if(likely(compsky::mysql::assign_next_row__no_free(res,  &row,  &_fp))){
				break;
			}
			return false;
		case files_from_which::bash:
		{
			const qint64 _sz = files_from_bash.readLine(FP, MEDIA_FP_SZ);
			if (_sz == -1){
				// Error, most likely exhausted all results
				return false;
			}
			FP[_sz - 1] = 0; // Remove trailing newline
			_fp = FP;
			break;
		}
		default:
			return false;
	}
	
	printf("%s\n", _fp);
	
	return true;
}


int main(const int argc,  const char** argv){
	char buf[4096];
	
	MYSQL* mysql;
	constexpr static const size_t auth_sz = 512;
	char auth[auth_sz];
	compsky::mysql::init(mysql, auth, auth_sz, getenv("TAGEM_MYSQL_CFG"));
	
	InlistFilterRules r(mysql, buf);
	
	r.load(argv[1]);
	
	r.get_results(res, files_from_bash);
	
	while(iteration(r));
	
	compsky::mysql::wipe_auth(auth, auth_sz);
}
