#include "etk.h"
// #include "etk_app.h"

int main( int argc, char* argv[] ){ 
	etk_init(argc, argv);
	etk_app_main(argc,argv);
	etk_run();
	etk_exit();
	return 0;    
}