#include "stdio.h"
extern char* optarg;

int line_width = 24;
int read_height = 20;

void show_usage(int argc, char* argv[]) {
  printf(
      "hexdump - dump files\n"
      "usage: hexdump [path]\n"
      "\n");
}

void hexdump(char* name) {
  int size = line_width * read_height;
  char* buffer = malloc(size);
  memset(buffer, 0, size);
  FILE* fp;
  fp = fopen(name, "r+");
  int offset = 0;
  for (;;) {
    fseek(fp, offset, SEEK_SET);
    int ret = fread(buffer, size, 1, fp);
    if (ret <= 0) {
      break;
    }
    for (int i = 0; i < ret; i++) {
      if (i % line_width == 0) printf("\n %07x   ", offset);
      printf("%02x ", 0xff & buffer[i]);
      offset++;
    }
  }
  printf("\n");
  if (fp != NULL) {
    fclose(fp);
  }
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    int c;
    while ((c = getopt(argc, argv, "wh?")) != -1) {
      switch (c) {
        case 'w':
          line_width = atoi(optarg);
          break;
        case 'h':
          read_height = atoi(optarg);
          break;
          break;
        case '?':
          show_usage(argc, argv);
          return 0;
      }
    }
    printf("dump %s\n", argv[1]);
    hexdump(argv[1]);
  } else {
    show_usage(argc, argv);
  }
  return 0;
}