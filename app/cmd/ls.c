#include "stdio.h"
#include <dirent.h>

char *buf="hello,ls\n";
int main(int argc, char* argv[]) {
    printf(buf);
    DIR * dir;
    struct dirent * ptr;
    int i;
    dir = opendir("/");
    while((ptr = readdir(dir)) != NULL){
        printf("name : %s type: %d\n", ptr->d_name,ptr->d_type);
    }
    closedir(dir);
    return 0;
}