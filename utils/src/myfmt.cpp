/*
Read newline-seperated file paths from STDIN and write directory-aggregated filenames

E.g.
    ./myfindsvid /tmp | ./mysort | sort -n --reverse | sed -r 's/^([0-9]+\t)+//g' | tr -d '\n' | ./myfmt
*/

#include <unistd.h> // for read, write
#include <string.h> // for strlen, memcpy
#include <stdio.h> // for getline, stdin
#include <stdlib.h> // for malloc

const int zero = 0;


void write_dir(const char* fp, const int dir_len){
    write(STDOUT_FILENO,  &zero,  sizeof(int));
    write(STDOUT_FILENO,  &dir_len,  sizeof(int));
    write(STDOUT_FILENO,  fp,  dir_len);
}

int main(const int argc, const char* argv[]){
    int dir_len;
    char fp[4096];
    char olddir[1024];
    int olddir_len = 0;
    int dir_match;
    
    olddir[0] = 0;
    
    while (true){
        if (getline(&fp, 4096, stdin) == -1)
            break;
        
        int fp_len = strlen(fp) - 1;
        
        dir_match = 0;
        for (auto i = 0;  i < fp_len;  ++i){
            if (fp[i] == '/')
                dir_len = i;
            if (i < olddir_len && fp[i] == olddir[i])
                ++dir_match;
        }
        
        if (dir_len != olddir_len  ||  dir_match != olddir_len)
            write_dir(fp, dir_len);
         
        int fname_len = fp_len - 1 - dir_len;
        write(STDOUT_FILENO,  &fname_len,  sizeof(int));
        write(STDOUT_FILENO,  fp + dir_len + 1,  fname_len);
        
        memcpy(olddir,  fp,  dir_len);
        olddir[dir_len] = 0;
        olddir_len = dir_len;
    }
    free(fp);
}
