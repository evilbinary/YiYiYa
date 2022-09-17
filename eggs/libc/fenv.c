#include "fenv.h"


#define UNIMPL() printf("unimpl %s:%d %s\n",__FILE__,__LINE__,__FUNCTION__)

int fesetround(int r){
    UNIMPL();
	return 0;
}