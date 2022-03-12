#include <sys/stat.h>

#include "dirent.h"
#include "stdio.h"

int show_all = 0;
int long_mode = 0;
int show_nice = 0;
char buf[512];

void show_usage(int argc, char* argv[]) {
  printf(
      "ls - list files\n"
      "usage: %s [-lha] [path]\n"
      " -a     \033[3mlist all files (including . files)\033[0m\n"
      " -l     \033[3muse a long listing format\033[0m\n"
      " -h     \033[3mhuman-readable file sizes\033[0m\n"
      " -?     \033[3mshow this help text\033[0m\n"
      "\n",
      argv[0]);
}

void ls(char* path) {
  DIR* dir;
  struct dirent* ptr;
  struct stat mystat;
  int i;
  dir = opendir(path);
  while ((ptr = readdir(dir)) != NULL) {
    if (long_mode == 1) {
      printf("%-20s", ptr->d_name);
      if (ptr->d_type == DT_DIR) {
        printf("dir");
      }
      if (ptr->d_type == DT_REG) {
        printf("file");
      } else {
        printf("%x", ptr->d_type);
      }
      sprintf(buf, "%s/%s", path, ptr->d_name);
      stat(buf, &mystat);
      printf("%d", mystat.st_size);

      printf("\n");
    } else {
      printf("%-18s", ptr->d_name);
      if (i % 6 == 0) {
        printf("\n");
      }
      i++;
    }
  }
  printf("\n");
  closedir(dir);
}

int main(int argc, char* argv[]) {
  char* path = "/";
  if (argc > 1) {
    int c;
    while ((c = getopt(argc, argv, "ahl?")) != -1) {
      switch (c) {
        case 'a':
          show_all = 1;
          break;
        case 'h':
          show_nice = 1;
          break;
        case 'l':
          long_mode = 1;
          break;
        case '?':
          show_usage(argc, argv);
          return 0;
      }
    }
  }
  ls(path);

  return 0;
}