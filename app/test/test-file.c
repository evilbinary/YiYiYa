#include "stdio.h"
// #include <sys/types.h>
#include <dirent.h>
// #include <unistd.h>

char* buf = "hello,file\n";

void test_read() {
  char* buffer = malloc(512);
  memset(buffer, 0, 512);
  FILE* fp;
  fp = fopen("/duck.jpg", "r+");
  printf("fd=%d\n", *fp);
  int offset = 0;
  for (;;) {
    fseek(fp, offset, SEEK_SET);
    int ret = fread(buffer, 512, 1, fp);
    if (ret <= 0) {
      break;
    }
    printf("ret=>%d\n", ret);
    for (int i = 0; i < 512; i++) {
      if (i % 24 == 0) printf("\n %x   ", offset);
      printf("%x ", 0xff & buffer[i]);
      offset++;
    }
  }
  printf("\n");
  if (fp != NULL) {
    fclose(fp);
  }
}

void test_write() {
  FILE* fp= fopen("/kaka.txt", "w+");
  fseek(fp, 0, SEEK_SET);
  fwrite("ABCDEF",strlen("ABCDEF"),1,fp);
  fclose(fp);
}

void test_read_dir(){
    DIR * dir;
    struct dirent * ptr;
    int i;
    dir = opendir("/hello");
    while((ptr = readdir(dir)) != NULL){
        printf("name : %s type: %d\n", ptr->d_name,ptr->d_type);
    }
    closedir(dir);
}

int main(int argc, char* argv[]) {
  printf(buf);
  test_write();
  // test_read();
  // test_read_dir();
  return 0;
}