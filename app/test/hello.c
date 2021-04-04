#include "ioctl.h"
#include "stdio.h"
#include "types.h"
#include "unistd.h"

#define IOC_PTY_MAGIC 'p'

#define IOC_SLAVE _IOW(IOC_PTY_MAGIC, 1, int)
#define IOC_READ_OFFSET _IOW(IOC_PTY_MAGIC, 3, int)
#define IOC_WRITE_OFFSET _IOW(IOC_PTY_MAGIC, 4, int)

char* buf = "hello,elf\n";

void test_pty() {
  u32 fd_ptm, fd_pts;
  fd_ptm = open("/dev/ptm", "r");
  if (fd_ptm < 0) {
    printf("error get ptm \n");
  }
  u32 pts = ioctl(fd_ptm, IOC_SLAVE);
  char buf[20];
  sprintf(buf, "/dev/pts/%d", pts);
  fd_pts = open(buf, "r");
  if (fd_pts < 0) {
    printf("error get pts \n");
  }
  printf("ptm %d pts %d\n", fd_ptm, fd_pts);

  char* buf2 = "abcdef";
  int len = strlen(buf2);
  write(fd_ptm, buf2, len);
  memset(buf2, 0, len);
  read(fd_pts, buf2, len);
  printf("read from ptm: %s\n", buf2);
}

void test_fork() {
  printf("fork\n");
  u32 i=0;
  pid_t fpid = fork();
  printf("fork end %d\n",fpid);
  if (fpid == 0) {
    printf("child %d %d %d\n", getppid(), getpid(), fpid);
  } else {
    printf("parent %d %d %d\n", getppid(), getpid(), fpid);
  }
  for(int i=0;i<10;i++){
      printf("%d %d| ",getpid(),i++);
      // getpid();
  }
}
int main(int argc, char* argv[]) {
  printf(buf);
  test_fork();
  return 0;
}