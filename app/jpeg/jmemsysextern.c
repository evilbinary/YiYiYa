#include "jpeglib.h"


void * jpeg_get_small JPP((j_common_ptr cinfo, size_t sizeofobject)){
	//lcd_printf("jpeg_get_small:%d\n",sizeofobject);
	return (void *)malloc(sizeofobject);
}
void jpeg_free_small JPP((j_common_ptr cinfo, void * object, size_t sizeofobject)){
	//lcd_printf("jpeg_free_small:%d\n",sizeofobject);
	free(object);
}

long jpeg_mem_init JPP((j_common_ptr cinfo)){

	return 1;
}
void jpeg_mem_term JPP((j_common_ptr cinfo)){

}
void*  jpeg_get_large JPP((j_common_ptr cinfo, size_t sizeofobject)){
	//lcd_set_pos(0,0);
	//lcd_printf("jpeg_get_large\n");
	return (void *)malloc(sizeofobject);					       
}
void jpeg_free_large JPP((j_common_ptr cinfo, void FAR * object, size_t sizeofobject)){
	//lcd_set_pos(100,0);
	//lcd_printf("jpeg_free_large\n");
	// printf("jpeg_free_large\n");
	free(object);
}
