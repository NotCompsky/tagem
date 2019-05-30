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

#include "mymysql.hpp" // for mymysql::*

namespace res1 {
    #include "mymysql_results.hpp" // for ROW, RES, COL, ERR
}



constexpr const int BUF_SZ_INIT = 4096;
const int BUF_SZ = BUF_SZ_INIT;
char* BUF = (char*)malloc(BUF_SZ_INIT);
int BUF_INDX;



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
    asciify("\n\t\t(\n\t\t\tSELECT DISTINCT f2t.file_id\n\t\t\tFROM file2tag f2t\n\t\t\tWHERE f2t.tag_id\n\t\t\tIN (SELECT node FROM _tmp_tagids_", TMPTBL_POSTFIX, ')');
    
    SQL__CALL_DESC_TAGS_FROM[strlen("CALL descendant_tags_id_from(\"_tmp_tagids_")] = TMPTBL_POSTFIX;
    
    increment_tmptbl_postfix();
}

void add_union_end(){
    SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX++] = '"';
    SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX++] = ')';
    SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX] = 0;
    
    fwrite(SQL__CALL_DESC_TAGS_FROM, 1, SQL__CALL_DESC_TAGS_FROM__INDX, stderr);
    fwrite("\n", 1, 1, stderr);
    
    mymysql::exec((const char*)SQL__CALL_DESC_TAGS_FROM);
    
    SQL__CALL_DESC_TAGS_FROM__INDX = strlen("CALL descendant_tags_id_from(\"_tmp_tagids_a\",  \"");
    
    asciify("\n\t\t)\n\t");
}

void add_union_unionness(){
    asciify("\n\t\t\tUNION ALL");
}

void construct_stmt(const char** argv){
    BUF_INDX = 0;
    
    bool add_comma = false;
    
    
    asciify("SELECT f.name\nFROM file f\nJOIN (\n\tSELECT file_id FROM (\n\t\t"); 
    
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
    
    asciify(") AS u\n\tGROUP BY file_id\n\tHAVING count(*) >= ",  count_str,  "\n) F ON f.id = F.file_id");
}


int main(const int argc, const char* argv[]){
    mymysql::init(argv[1]);
    
    construct_stmt(argv + 2);
    
    BUF[BUF_INDX] = '\n';
    fwrite(BUF, 1, BUF_INDX+1, stderr);
    BUF[BUF_INDX] = 0;
    
    res1::query();
    
    BUF_INDX = 0;
    
    char* fp;
    while (res1::assign_next_result(&fp)){
        const int i = strlen(fp);
        if (BUF_INDX + i + 1  >  BUF_SZ){
            fwrite(BUF, 1, BUF_INDX, stderr);
            BUF_INDX = 0;
        }
        memcpy(BUF + BUF_INDX,  fp,  i);
        BUF_INDX += i;
        BUF[BUF_INDX++] = '\n';
    }
    
    fwrite(BUF, 1, BUF_INDX, stderr);
}
