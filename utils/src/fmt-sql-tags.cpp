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


#include <unistd.h> // for write
#include <string.h> // for strlen, memcpy?

#include "sql_utils.hpp" // for mysu::*, SQL_*


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

int add_union_start(char* stmt, int i){
    SQL__CALL_DESC_TAGS_FROM[strlen("CALL descendant_tags_id_from(\"_tmp_tagids_")] = TMPTBL_POSTFIX;
    
    const char* a = "\n\t\t(\n\t\t\tSELECT DISTINCT f2t.file_id\n\t\t\tFROM file2tag f2t\n\t\t\tWHERE f2t.tag_id\n\t\t\tIN (SELECT node FROM _tmp_tagids_";
    memcpy(stmt + i,  a,  strlen(a));
    i += strlen(a);
    
    stmt[i++] = TMPTBL_POSTFIX;
    
    stmt[i++] = ')';
    
    increment_tmptbl_postfix();
    
    
    return i;
}

int add_union_end(char* stmt, int i){
    SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX++] = '"';
    SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX++] = ')';
    SQL__CALL_DESC_TAGS_FROM[SQL__CALL_DESC_TAGS_FROM__INDX] = 0;
    
    write(2, SQL__CALL_DESC_TAGS_FROM, SQL__CALL_DESC_TAGS_FROM__INDX);
    write(2, "\n", 1);
    SQL_STMT->execute(SQL__CALL_DESC_TAGS_FROM);
    
    SQL__CALL_DESC_TAGS_FROM__INDX = strlen("CALL descendant_tags_id_from(\"_tmp_tagids_a\",  \"");
    
    
    const char* a = "\n\t\t)\n\t";
    memcpy(stmt + i,  a,  strlen(a));
    i += strlen(a);
    
    return i;
}

int add_union_unionness(char* stmt, int i){
    const char* a = "\n\t\t\tUNION ALL";
    memcpy(stmt + i,  a,  strlen(a));
    i += strlen(a);
    
    return i;
}

int construct_stmt(char* stmt, const char** argv){
    // returns final strlen(stmt)
    int i;
    bool add_comma = false;
    int n_unions = 0;
    
    i = 0;
    
    const char* a = "SELECT f.name\nFROM file f\nJOIN (\n\tSELECT file_id FROM (\n\t\t";
    memcpy(stmt + i,  a,  strlen(a));
    i += strlen(a);
    
    i = add_union_start(stmt, i);
    
    const char* count_str = *(argv++);
    
    while (*argv != 0){
        const char* arg = *(argv++);
        
        if (arg[1] == 0){
            switch(arg[0]){
                case '&':
                    ++n_unions;
                    i = add_union_end(stmt, i);
                    i = add_union_unionness(stmt, i);
                    add_comma = false;
                    i = add_union_start(stmt, i);
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
    
    //if (n_unions != 0)
    //    i -= strlen("UNION ALL ");
    
    i = add_union_end(stmt, i);
    
    const char* b = ") AS u\n\tGROUP BY file_id\n\tHAVING count(*) >= ";
    memcpy(stmt + i,  b,  strlen(b));
    i += strlen(b);
    
    memcpy(stmt + i,  count_str,  strlen(count_str));
    i += strlen(count_str);
    
    const char* c = "\n) F ON f.id = F.file_id";
    memcpy(stmt + i,  c,  strlen(c));
    i += strlen(c);
    
    stmt[i++] = ';';
    stmt[i++] = '\n';
    stmt[i] = 0;
    
    return i;
}

const char endline = '\n';

int main(const int argc, const char* argv[]){
    char stmt[1 << 20];
    
    mysu::init(argv[1], "mytag");
    
    int i = construct_stmt(stmt, argv + 2);
    
    write(2, stmt, strlen(stmt));
    
    SQL_RES = SQL_STMT->executeQuery(stmt);
    
    while (SQL_RES->next()){
        std::string res = SQL_RES->getString(1);
        const char* res_cstr = res.c_str();
        write(1,  res_cstr,  strlen(res_cstr));
        write(1,  &endline,  1);
    }
}
