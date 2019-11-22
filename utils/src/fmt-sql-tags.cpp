/*
Convert    ./fmt-sql-tags 2 foo bar '&' ree gee dee '&' chi phi khi

To

CALL descendant_tags_id_from("_tmp_tagids_a",  "'foo', 'bar'");
CALL descendant_tags_id_from("_tmp_tagids_b",  "'ree', 'gee', 'dee'");
CALL descendant_tags_id_from("_tmp_tagids_c",  "'chi', 'phi', 'khi'");

SELECT f.name
FROM file f
JOIN (
    SELECT file_id
    FROM (
        (
            SELECT DISTINCT f2t.file_id
            FROM file2tag f2t
            WHERE f2t.tag_id IN (SELECT node FROM _tmp_tagids_a)
        )
        UNION ALL
        (
            SELECT DISTINCT f2t.file_id
            FROM file2tag f2t
            WHERE f2t.tag_id IN (SELECT node FROM _tmp_tagids_b)
        )
        UNION ALL
        (
            SELECT DISTINCT f2t.file_id
            FROM file2tag f2t
            WHERE f2t.tag_id IN (SELECT node FROM _tmp_tagids_c)
        )
    )
    AS u
    GROUP BY file_id
    HAVING count(*) >= 2
) F ON f.id = F.file_id
;


TODO: Aim is to include other tables, not just tags, and allow arbitrary deeply nested logic.
*/


#include <stdio.h> // for fwrite
#include <string.h> // for strlen, memcpy?
#include <cstdlib> // for malloc

#include <compsky/mysql/mysql.hpp> // for compsky::mysql::*
#include <compsky/mysql/query.hpp> // for ROW, RES, COL, ERR


MYSQL_RES* RES;
MYSQL_ROW ROW;

namespace compsky::asciify {
    char* BUF;
	char* ITR;
    constexpr static const size_t BUF_SZ = 4096;
        
    void ensure_buf_can_fit(size_t n){
        if (get_index(BUF, ITR) + n  >  BUF_SZ){
            fwrite(BUF, 1, get_index(BUF, ITR), stderr);
            ITR = BUF;
        }
    }
}

namespace _f {
    constexpr static const compsky::asciify::flag::StrLen strlen;
}



char TMPTBL_POSTFIX = '0'; // Lazy placeholder - supports a mere 62 tables. // TODO: Replace with arbitray string or integer thing
char SQL__CALL_DESC_TAGS_FROM[1024] = "CALL descendant_tags_id_from(\"_tmp_tagids_a\",  \"";
int SQL__CALL_DESC_TAGS_FROM__INDX = strlen("CALL descendant_tags_id_from(\"_tmp_tagids_a\",  \"");


void increment_tmptbl_postfix(){
    if (TMPTBL_POSTFIX == '9')
        TMPTBL_POSTFIX = 'a';
    else if (TMPTBL_POSTFIX == 'z')
        TMPTBL_POSTFIX = 'A';
    else if (TMPTBL_POSTFIX == 'Z')
        exit(2);
    else
        ++TMPTBL_POSTFIX;
}

void add_union_start(){
    compsky::asciify::asciify(ITR, "\n\t\t(\n\t\t\tSELECT DISTINCT f2t.file_id\n\t\t\tFROM file2tag f2t\n\t\t\tWHERE f2t.tag_id\n\t\t\tIN (SELECT node FROM _tmp_tagids_", TMPTBL_POSTFIX, ')');
    
    SQL__CALL_DESC_TAGS_FROM[strlen("CALL descendant_tags_id_from(\"_tmp_tagids_")] = TMPTBL_POSTFIX;
    
    increment_tmptbl_postfix();
}

void add_union_end(){
    SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX++] = '"';
    SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX++] = ')';
    SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX] = '\n';
    
    fwrite(SQL__CALL_DESC_TAGS_FROM, 1, SQL__CALL_DESC_TAGS_FROM__INDX, stderr);
	fwrite(";\n", 2, 1, stderr);
    
    compsky::mysql::exec_buffer((const char*)SQL__CALL_DESC_TAGS_FROM,  SQL__CALL_DESC_TAGS_FROM__INDX);
    
    SQL__CALL_DESC_TAGS_FROM__INDX = strlen("CALL descendant_tags_id_from(\"_tmp_tagids_a\",  \"");
    
    compsky::asciify::asciify(ITR, "\n\t\t)\n\t");
}

void add_union_unionness(){
    compsky::asciify::asciify(ITR, "\n\t\t\tUNION ALL");
}

void construct_stmt(const char** argv,  const char* score_condition){
    ITR = BUF;
    
    bool add_comma = false;
    
    
    compsky::asciify::asciify(ITR, "SELECT f.name\nFROM file f\nJOIN (\n\tSELECT file_id FROM (\n\t\t"); 
    
    add_union_start();
    
    const char* count_str = *argv;
    
    while (*(++argv) != 0){
        const char* arg = *argv;
        
        if (arg[1] == 0){
            switch(arg[0]){
                case '&':
                    add_union_end();
                    add_union_unionness();
                    add_comma = false;
                    add_union_start();
                    goto goto__next_arg;
                default: break;
            }
        }
        
        if (add_comma)
            SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX++] = ',';
        else
            add_comma = true;
        
        SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX++] = '\'';
        
        memcpy(SQL__CALL_DESC_TAGS_FROM + SQL__CALL_DESC_TAGS_FROM__INDX,  arg,  strlen(arg));
        SQL__CALL_DESC_TAGS_FROM__INDX += strlen(arg);
        
        SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX++] = '\'';
        
        
        
        
        
        goto__next_arg:
        ; // Dummy statement
    }
    
    add_union_end();
    
    compsky::asciify::asciify(ITR, ") AS u\n\tGROUP BY file_id\n\tHAVING count(*) >= ",  count_str,  "\n) F ON f.id = F.file_id");
	compsky::asciify::asciify(ITR, 
		(score_condition) ? " WHERE f.score " : "",
		(score_condition) ? score_condition : ""
	);
}


int main(const int argc,  const char** argv){
	if(compsky::asciify::alloc(compsky::asciify::BUF_SZ))
		return 4;
	
    compsky::mysql::init(getenv("TAGEM_MYSQL_CFG"));
	
	const char* score_condition = nullptr;
	while(true){
		// Everything before "--" is considered an option/flag/etc, not an argument
		const char* arg = *(++argv);
		if(arg[0] != '-'  ||  arg[2] != 0)
			break;
		switch(arg[1]){
			case 's':
				score_condition = *(++argv);
				break;
			default: exit(3);
		}
	}
    
    construct_stmt(argv, score_condition);
    
    compsky::asciify::BUF[compsky::asciify::get_index(BUF, ITR)] = '\n';
    fwrite(compsky::asciify::BUF, 1, compsky::asciify::get_index(BUF, ITR)+1, stderr);
    
    compsky::mysql::query_buffer(&RES, compsky::asciify::BUF, compsky::asciify::get_index(BUF, ITR));
    
    ITR = BUF;
    
    char* fp;
    while (compsky::mysql::assign_next_row(RES, &ROW, &fp)){
        const size_t fp_sz = strlen(fp);
        compsky::asciify::ensure_buf_can_fit(fp_sz + 1);
        compsky::asciify::asciify(ITR, _f::strlen, fp, fp_sz, '\n');
    }
    
    fwrite(compsky::asciify::BUF, 1, compsky::asciify::get_index(BUF, ITR), stdout);
}
