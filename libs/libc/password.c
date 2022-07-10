#include "pwd.h"
#include "stdio.h"

#define UNIMPL() printf("unimpl %s:%d %s\n",__FILE__,__LINE__,__FUNCTION__)


struct passwd *getpwuid (uid_t t){
  UNIMPL();

  return NULL;
}
struct passwd *getpwnam (const char * name){
  UNIMPL();

  return NULL;
}