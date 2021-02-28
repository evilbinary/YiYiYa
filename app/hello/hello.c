#include "stdio.h"
#include "syscall.h"

int main(int argc, char* argv[]) {
    char buf[512]={"hello,elf\n"};
    // syscall1(SYS_TEST, buf);
    syscall0(SYS_TEST);
    // printf("hello,elf\n");
    for(;;){
        //printf("hello,elf\n");
        syscall0(SYS_TEST);
    }
    return 0;
}