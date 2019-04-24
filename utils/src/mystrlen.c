#include <unistd.h> // for read, write

int main(const int argc, const char* argv[]){
    int i;
    for (int j = 0;  j < argc;  ++j){
        for (i = 0;  argv[j][i] != 0;  ++i);
        write(STDOUT_FILENO,  &i,  sizeof(int));
        write(STDOUT_FILENO,  argv[j][i],  i);
    }
}
