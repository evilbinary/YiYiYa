#include "etk_image_decoder.h"

//Ret etk_image_decoder_match(EtkImageDecoder* thiz, const char* filename){
//	return thiz->match(thiz,filename);
//}
EtkBitmap* etk_image_decoder_decode(EtkImageDecoder* thiz, const char* filename){
	return thiz->decode(thiz,filename);
}
void etk_image_decoder_destroy(EtkImageDecoder* thiz){
	thiz->destroy(thiz);
}