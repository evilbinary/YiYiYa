#include "ioctl.h"
#include "stdio.h"
#include "stdlib.h"
#include "types.h"
#include "unistd.h"

#define IOC_PTY_MAGIC 'p'

#define IOC_SLAVE _IOW(IOC_PTY_MAGIC, 1, int)
#define IOC_READ_OFFSET _IOW(IOC_PTY_MAGIC, 3, int)
#define IOC_WRITE_OFFSET _IOW(IOC_PTY_MAGIC, 4, int)

char* buf = "hello test elf\n";

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
  u32 i = 0;
  pid_t fpid = fork();
  printf("fork end %d\n", fpid);
  if (fpid == 0) {
    printf("child %d %d %d\n", getppid(), getpid(), fpid);
  } else {
    printf("parent %d %d %d\n", getppid(), getpid(), fpid);
  }
  for (int i = 0; i < 10; i++) {
    printf("%d %d| ", getpid(), i++);
    // getpid();
  }
}

void test_read_write() {
  printf("fork\n");
  u32 fd_ptm, fd_pts;
  fd_ptm = open("/dev/ptm", "r");
  if (fd_ptm < 0) {
    printf("error get ptm \n");
  }
  u32 i = 0;
  pid_t fpid = fork();
  printf("fork end %d\n", fpid);
  if (fpid == 0) {
    printf("child %d %d %d\n", getppid(), getpid(), fpid);
    u32 pts = ioctl(fd_ptm, IOC_SLAVE);
    char buf[20];
    sprintf(buf, "/dev/pts/%d", pts);
    fd_pts = open(buf, "r");
    if (fd_pts < 0) {
      printf("error get pts \n");
    }
    int j = 0;
    int pid = getpid();
    for (;;) {
      char buf2[64];
      int len = 1;
      memset(buf2, 0, len);
      read(fd_pts, buf2, len);
      printf("child pid:%d j:%d read from ptm: %s\n", pid, j++, buf2);
    }
  } else {
    printf("parent %d %d %d\n", getppid(), getpid(), fpid);
    int j = 0;
    int pid = getpid();
    for (;;) {
      char* buf2 = "abcdef";
      int len = strlen(buf2);
      if (j < 2 || j > 1000000) {
        printf("parent pid:%d j:%d write ptm: %s\n", pid, j, buf2);
        write(fd_ptm, buf2, len);
      }
      j++;

    }
  }
  for (int i = 0; i < 1000000; i++) {
    printf("%d %d| ", getpid(), i++);
    // getpid();
  }
}

void test_dup_pty() {
  printf("fork\n");
  u32 fd_ptm, fd_pts;
  fd_ptm = open("/dev/ptm", "r");
  if (fd_ptm < 0) {
    printf("error get ptm \n");
  }
  u32 i = 0;
  pid_t fpid = fork();
  printf("fork end %d\n", fpid);
  if (fpid == 0) {
    printf("child %d %d %d\n", getppid(), getpid(), fpid);
    u32 pts = ioctl(fd_ptm, IOC_SLAVE);
    char buf[20];
    sprintf(buf, "/dev/pts/%d", pts);
    fd_pts = open(buf, "r");
    if (fd_pts < 0) {
      printf("error get pts \n");
    }
    int j = 0;
    int pid = getpid();
    for (;;) {
      char buf2[64];
      int len = 1;
      memset(buf2, 0, len);
      read(fd_pts, buf2, len);
      printf("pid:%d %d read from ptm: %s\n", pid, j++, buf2);
    }
  } else {
    printf("parent %d %d %d\n", getppid(), getpid(), fpid);
    int j = 0;
    int pid = getpid();
    for (;;) {
      char* buf2 = "abcdef";
      int len = strlen(buf2);
      if (j < 2 || j > 10000000) {
        printf("pid:%d %d write ptm: %s\n", pid, j, buf2);
        write(fd_ptm, buf2, len);
        j++;
      }
    }
  }
  for (int i = 0; i < 10; i++) {
    printf("%d %d| ", getpid(), i++);
    // getpid();
  }
}

void test_pipe() {
  pid_t pid;
  char buf[1024];
  int fd[2];
  char* p = "test for pipe\n";

  if (pipe(fd) == -1) printf("pipe error\n");
  printf(" fd0 %d fd1 %d\n", fd[0], fd[1]);
  pid = fork();
  if (pid < 0) {
    printf("fork err pid %d\n", getpid());
  } else if (pid == 0) {  // child
    printf("child pid %d\n", getpid());
    close(fd[1]);
    printf("start read\n");
    int len = read(fd[0], buf, strlen(p));
    printf("child read====>%s ret:%d\n", buf, len);

    memset(buf, 0, 1024);
    len = read(fd[0], buf, strlen(p));
    printf("child read2====>%s ret:%d\n", buf, len);

    // write(STDOUT_FILENO, buf, len);
    close(fd[0]);
  } else {  // parent
    printf("parent pid %d\n", getpid());
    close(fd[0]);
    for (int i = 0; i < 10000000; i++)
      ;
    write(fd[1], p, strlen(p));
    for (int i = 0; i < 10000000; i++)
      ;
    write(fd[1], "ABCDEF", strlen(p));
    close(fd[1]);
  }
  for (;;) {
    printf("%d ", getpid());
  }
}

void test_dup() {
  FILE *fp = fopen("/dev/sda/testdup.txt", "w");
  printf("open fp %x\n",fp);
  int fno=fileno(fp);
  fno=STDOUT_FILENO;

  int fd=dup(fno);
  printf("dup fd %d\n",fd);

  FILE* nfp=fdopen(fd,"w");
  fprintf(nfp,"test dup\n");
  close(fd);
}

void test_dup2() {
  int fd;
  fd = open("/dev/sda/testdup2.txt", "w");
  if (fd < 0) {
    printf("open error\n");
    exit(-1);
  }
  if (dup2(fd, STDOUT_FILENO) < 0) {
    printf("err in dup2\n");
  }
  printf("test str\n");
  close(fd);
}

void test_exec(){
  char * argv[] = {"ls", "-al", "/etc/passwd",};
  // execv("/bin/ls", argv);
  execv("/dev/sda/hello.elf", argv);
}

int main(int argc, char* argv[]) {
  printf(buf);
  // test_fork();
  // test_pty();
  // test_dup();
  // test_dup2();
  // test_pipe();
  // test_exec();
  // test_dup_pty();
  test_read_write();
  return 0;
}