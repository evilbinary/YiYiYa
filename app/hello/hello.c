#include "stdio.h"
#include "syscall.h"

int main(int argc, char* argv[]) {
    char buf[512]={"hello,elf\n"};
    syscall1(SYS_PRINT, "hello,elf\n");
    // syscall0(SYS_TEST);
    // printf(buf);
    for(;;){
        //printf("hello,elf\n");
        // syscall0(SYS_TEST);
    }
    return 0;
}