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


int main(int argc, char* argv[]) {
  printf("hello musl\n");
  test_malloc();
  // test_float();

  return 0;
}