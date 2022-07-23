#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

int main(int argc, char* argv[]) {
  char * format = "%Y-%m-%d %H:%M:%S";
	struct tm * timeinfo;
	struct timeval now;
	char buf[256] = {0};


  gettimeofday(&now, NULL);  // time(NULL);
  timeinfo = localtime((time_t*)&now.tv_sec);

  strftime(buf, BUFSIZ, format, timeinfo);
  printf("%s\n",buf);

  return 0;
}