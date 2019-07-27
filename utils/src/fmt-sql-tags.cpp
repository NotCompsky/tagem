/*
Convert    ./fmt-sql-tags [MYSQL_CFG] 2 foo bar '&' ree gee dee '&' chi phi khi

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
    char* BUF = (char*)malloc(4096);
    int BUF_SZ = 4096;
        
    void ensure_buf_can_fit(size_t n){
        if (get_index() + n  >  BUF_SZ){
            fwrite(BUF, 1, get_index(), stderr);
            reset_index();
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
    compsky::asciify::asciify("\n\t\t(\n\t\t\tSELECT DISTINCT f2t.file_id\n\t\t\tFROM file2tag f2t\n\t\t\tWHERE f2t.tag_id\n\t\t\tIN (SELECT node FROM _tmp_tagids_", TMPTBL_POSTFIX, ')');
    
    SQL__CALL_DESC_TAGS_FROM[strlen("CALL descendant_tags_id_from(\"_tmp_tagids_")] = TMPTBL_POSTFIX;
    
    increment_tmptbl_postfix();
}

void add_union_end(){
    SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX++] = '"';
    SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX++] = ')';
    SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX] = '\n';
    
    fwrite(SQL__CALL_DESC_TAGS_FROM, 1, SQL__CALL_DESC_TAGS_FROM__INDX, stderr);
    
    compsky::mysql::exec_buffer((const char*)SQL__CALL_DESC_TAGS_FROM,  SQL__CALL_DESC_TAGS_FROM__INDX);
    
    SQL__CALL_DESC_TAGS_FROM__INDX = strlen("CALL descendant_tags_id_from(\"_tmp_tagids_a\",  \"");
    
    compsky::asciify::asciify("\n\t\t)\n\t");
}

void add_union_unionness(){
    compsky::asciify::asciify("\n\t\t\tUNION ALL");
}

void construct_stmt(const char** argv){
    compsky::asciify::reset_index();
    
    bool add_comma = false;
    
    
    compsky::asciify::asciify("SELECT f.name\nFROM file f\nJOIN (\n\tSELECT file_id FROM (\n\t\t"); 
    
    add_union_start();
    
    const char* count_str = *(argv++);
    
    while (*argv != 0){
        const char* arg = *(argv++);
        
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
    
    compsky::asciify::asciify(") AS u\n\tGROUP BY file_id\n\tHAVING count(*) >= ",  count_str,  "\n) F ON f.id = F.file_id");
}


int main(const int argc,  const char** argv){
    compsky::mysql::init(getenv("TAGEM_MYSQL_CFG"));
    
    construct_stmt(argv + 1);
    
    compsky::asciify::BUF[compsky::asciify::get_index()] = '\n';
    fwrite(compsky::asciify::BUF, 1, compsky::asciify::get_index()+1, stderr);
    
    compsky::mysql::query_buffer(&RES, compsky::asciify::BUF, compsky::asciify::get_index());
    
    compsky::asciify::reset_index();
    
    char* fp;
    while (compsky::mysql::assign_next_row(RES, &ROW, &fp)){
        const size_t fp_sz = strlen(fp);
        compsky::asciify::ensure_buf_can_fit(fp_sz + 1);
        compsky::asciify::asciify(_f::strlen, fp, fp_sz, '\n');
    }
    
    fwrite(compsky::asciify::BUF, 1, compsky::asciify::get_index(), stderr);
}
