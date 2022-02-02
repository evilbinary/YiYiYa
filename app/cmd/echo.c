#include <unistd.h>

#include "stdio.h"

// #define DEBUG

int main(int argc, char* argv[]) {
  if (argc > 1) {
#ifdef DEBUG
    printf("argc %d\n", argc);
    for (int i = 0; i < argc; i++) {
      printf(" argv[%d]=%s\n", i, argv[i]);
    }
#endif
    printf("%s\n", argv[1]);
  }
}