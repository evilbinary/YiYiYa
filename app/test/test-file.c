#include "stdio.h"
// #include <sys/types.h>
#include <dirent.h>
// #include <unistd.h>

char* buf = "hello,file\n";

#define PRINT_WIDTH 24
#define READ_BUFFER 24 * 20

void test_read(char* name) {
  char* buffer = malloc(READ_BUFFER);
  memset(buffer, 0, READ_BUFFER);
  FILE* fp;
  fp = fopen(name, "r+");
  printf("fd=%d\n", *fp);
  int offset = 0;
  for (;;) {
    fseek(fp, offset, SEEK_SET);
    int ret = fread(buffer, READ_BUFFER, 1, fp);
    if (ret <= 0) {
      break;
    }
    for (int i = 0; i < ret; i++) {
      if (i % PRINT_WIDTH == 0) {
        printf("\n %07x   ", offset);
      }
      printf("%02x ", 0xff & buffer[i]);
      offset++;
    }
    printf("ret=%d\n", ret);
  }
  if (fp != NULL) {
    fclose(fp);
  }
}

void test_write() {
  FILE* fp = fopen("/hello.txt", "w+");
  fseek(fp, 0, SEEK_SET);
  fwrite("ABCDEF", strlen("ABCDEF"), 1, fp);
  fclose(fp);
}

void test_write_read() {
  FILE* fp = fopen("/hello.txt", "w+");
  fseek(fp, 0, SEEK_SET);
  for (int i = 0; i < 100; i++) {
    fwrite("ABCDEF", strlen("ABCDEF"), 1, fp);
  }
  fclose(fp);

  test_read("/hello.txt");
}

void test_read_dir() {
  DIR* dir;
  struct dirent* ptr;
  int i;
  dir = opendir("/");
  while ((ptr = readdir(dir)) != NULL) {
    printf("name : %s type: %d\n", ptr->d_name, ptr->d_type);
  }
  closedir(dir);
}

int main(int argc, char* argv[]) {
  printf(buf);
  // test_write();
  test_read("/duck.png");
  // test_write_read();
  // test_read_dir();
  return 0;
}