#include <dirent.h>
#include <math.h>

#include "stdio.h"

void test_malloc() {
  for (int i = 0; i < 100; i++) {
    void* p = malloc(4096);
    printf("%d addr:%x\n", i, p);
  }
}

int test_float(void) {
  double x = sqrt(2.0);
  printf("The sqrt of 2 is: %f\n", x);
  return 0;
}

void test_getenv() {
  char* p;
  if ((p = getenv("USER"))) printf("USER =%s\n", p);
  setenv("USER", "test", 1);
  printf("USER=%s\n", getenv("USER"));
  unsetenv("USER");
  printf("USER=%s\n", getenv("USER"));

  // return USER = root　USER = test　USER = (null)
}

void test_read_root() {
  DIR* dir;
  struct dirent* ptr;
  dir = opendir("/");
  while ((ptr = readdir(dir)) != NULL) {
    printf("name : %s type: %d   ", ptr->d_name, ptr->d_type);

  }
  closedir(dir);
}

static char get_u8(int fd) {
  char buf[1]={0xff};
  printf("get fd %d\n", fd);
  if (read(fd, &buf, 1) != 1) return -1;
  printf("  ret=>%x\n", buf[0]);
  return buf[0];
}

void test_read_byte() {
  char* path = "scheme.boot";
  int fd = open(path, 0);
  if (get_u8(fd) != 0 || get_u8(fd) != 0 || get_u8(fd) != 0 ||
      get_u8(fd) != 0 || get_u8(fd) != 'c' || get_u8(fd) != 'h' ||
      get_u8(fd) != 'e' || get_u8(fd) != 'z') {
    printf("malformed fasl-object header in %s\n", path);
  }
}

int main(int argc, char* argv[]) {
  printf("hello musl\n");
  // test_malloc();
  // test_float();
  // test_getenv();
  // test_read_root();
  test_read_byte();

  return 0;
}