#include "stdio.h"
#include <dirent.h>

int main(int argc, char* argv[]) {
    DIR * dir;
    struct dirent * ptr;
    int i;
    dir = opendir("/");
    while((ptr = readdir(dir)) != NULL){
        printf("%-20s", ptr->d_name);
        if(ptr->d_type==DT_DIR){
            printf("dir");
        }if(ptr->d_type==DT_REG){
            printf("file");
        }else{
            printf("%x",ptr->d_type);
        }
        printf("\n");
    }
    closedir(dir);
    return 0;
}