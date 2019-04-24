#include <stdlib.h> // for malloc, free
#include <unistd.h> // for read, write
#include <string.h> // for memcpy
#include <stdio.h> // for printf

#define STDIN_FILENO 0
#define ASCII_OFFSET 48


const char NEWLINE = '\n';
const char TAB = '\t';


int count_digits(int n){
    // Obviously for non-negative integers only
    int count = 0;
    while (n != 0){
        ++count;
        n /= 10;
    }
    return count;
}


int main(const int argc, const char* argv[]){
    int fp_len;
    int dir_len;
    char fname[1024];
    char fp[2048];
    char dir[1024];
    char writebuf[2048];
    
    //std::map<size_t, char**> fsize2fps;
    
    while (true){
        
        goto goto__readindfsd;
        
        while (fp_len == 0){
            // Change of directory
            // Might have multiple changes of directory before a filename is listed
            read(STDIN_FILENO,  &dir_len,  sizeof(int));
            read(STDIN_FILENO,  dir,  dir_len);
            
            memcpy(fp,  dir,  dir_len);
            fp[dir_len] = '/';
            
            goto__readindfsd:
            if (read(STDIN_FILENO,  &fp_len,  sizeof(int)) == 0)
                // Check the first read in order to see if pipe has reached end
                return 0;
        }
        
        //char* fp = (char*)malloc(fp_len);
        read(STDIN_FILENO,  fname,  fp_len);
        memcpy(fp + dir_len + 1,  fname,  fp_len);
        fp_len += dir_len + 1; // Previous value will be fname_len
        
        fp[fp_len] = 0;
        
        size_t fsize;
        read(STDIN_FILENO,  &fsize,  sizeof(size_t));
        
        int i;
        
        int n_digits = count_digits(fsize);
        i = n_digits;
        while (i != 0){
            writebuf[--i] = ASCII_OFFSET + (fsize % 10);
            fsize /= 10;
        }
        i = n_digits;
        
        writebuf[i++] = '\t';
        
        memcpy(writebuf + i,  &fp_len,  sizeof(int));
        i += sizeof(int);
        
        memcpy(writebuf + i,  fp,  fp_len);
        i += fp_len;
        
        writebuf[i++] = '\n';
        
        write(STDOUT_FILENO,  writebuf,  i);
    }
}
