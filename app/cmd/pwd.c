#include <unistd.h>

#include "stdio.h"

int main(int argc, char* argv[]) {
  char buf[256] = {0};
  char* test = getcwd(buf, 256);
  printf("%s\n", buf);
  return 0;
}