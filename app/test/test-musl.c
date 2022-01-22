#include <math.h>

#include "stdio.h"
#include <dirent.h>

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
  int i;
  dir = opendir("/");
  while ((ptr = readdir(dir)) != NULL) {
    printf("name : %s type: %d\n", ptr->d_name, ptr->d_type);
  }
  closedir(dir);
}

int main(int argc, char* argv[]) {
  printf("hello musl\n");
  // test_malloc();
  // test_float();
  // test_getenv();
  test_read_root();

  return 0;
}