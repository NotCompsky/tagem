#include <unistd.h> // for read, write

int main(const int argc, const char* argv[]){
    int i;
    for (i = 0;  argv[1][i] != 0;  ++i);
    write(STDOUT_FILENO,  &i,  sizeof(int));
}
