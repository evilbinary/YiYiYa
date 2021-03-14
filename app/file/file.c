#include "stdio.h"

char* buf = "hello,file\n";

int main(int argc, char* argv[]) {
  printf(buf);
  char* buffer = malloc(512);
  memset(buffer, 0, 512);
  FILE* fp;
  fp = fopen("/dev/sda/DUCK.JPG", "r+");
  printf("fd=%d\n", fp->fd);
  u32 offset = 0;
  for (;;) {
    fseek(fp, offset, SEEK_SET);
    u32 ret = fread(buffer, 512, 1, fp);
    if (ret <= 0) {
      break;
    }
    printf("ret=>%d\n",ret);
    for (int i = 0; i < 512; i++) {
      if(i%24==0) printf("\n %x   ",offset);
      printf("%x ", 0xff & buffer[i]);
      offset++;
    }
  }
  printf("\n");
  fclose(fp);

  return 0;
}