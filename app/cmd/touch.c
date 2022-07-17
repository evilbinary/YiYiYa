#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc > 1) {
    const char *path = (const char *)argv[1];
    int fd = open(path, O_CREAT | O_RDWR);
    if (fd < 0) {
      fprintf(stderr, "touch: fd %d error \n", fd);
      return 0;
    }

    close(fd);
  }else{
    printf("please input touch file\n");
  }
}