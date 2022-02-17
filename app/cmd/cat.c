#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  FILE *file;
  int chr;
  int count;

  for (count = 1; count < argc; count++) {
    if ((file = fopen(argv[count], "r")) == NULL) {
      printf("%s: %s : %s\n", argv[0], argv[count], strerror(errno));
      continue;
    }
    while ((chr = getc(file)) != EOF) printf("%c", chr);
    fclose(file);
  }
}