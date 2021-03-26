#include "etk_text_buffer.h"
#include "etk_platform.h"

EtkTextBuffer* etk_text_buffer_create(e32 init_length){
	EtkTextBuffer *thiz=(EtkTextBuffer*)ETK_MALLOC(sizeof(EtkTextBuffer));
	if(thiz!=NULL){
		thiz->length=0;
		thiz->buffer=(char*)ETK_MALLOC(init_length);
		thiz->buffer_length=init_length;
		memset(thiz->buffer,0,init_length);
		thiz->buffer[init_length]='\0';
	}
	return thiz;
}

static Ret etk_text_buffer_extend(EtkTextBuffer* thiz, e32 length){
	char* buffer = NULL;
	e32 buffer_length = 0;
	if((thiz->length + length) < thiz->buffer_length){
		return RET_OK;
	}
	buffer_length = (thiz->buffer_length + length + 10);
	buffer_length = buffer_length + (buffer_length>>1);

	if((buffer = (char*)ETK_REALLOC(thiz->buffer,buffer_length)) != NULL){
		thiz->buffer = buffer;
		thiz->buffer_length = buffer_length;
		thiz->buffer[thiz->length] = '\0';
		return RET_OK;
	}
	return RET_FAIL;
}

Ret   etk_text_buffer_insert(EtkTextBuffer* thiz, e32 offset, const char* text, e32 len){
	int str_len = len;
	size_t length = 0;
	char* dst = NULL;
	const char* src = NULL;
	if(!(thiz != NULL && thiz->buffer != NULL && text != NULL))
		return RET_FAIL;	
	str_len = len < 0 ? (int)strlen(text) : len;
	if(!(etk_text_buffer_extend(thiz, str_len + 1) == RET_OK)){
		return RET_FAIL;
	}
	if(!(offset <= thiz->length))
		return RET_FAIL;

	if(offset < thiz->length){
		length = thiz->length - offset;
		dst = thiz->buffer + thiz->length + str_len - 1;
		src = thiz->buffer + thiz->length - 1;
		for(; length != 0; length--, dst--, src--){
			*dst = *src;
		}
	}
	dst = thiz->buffer + offset;
	src = text;
	length = str_len;
	for(;length != 0; length--, dst++, src++){
		*dst = *src;
	}
	thiz->length += str_len;
	thiz->buffer[thiz->length] = '\0';
	//printf("buffer:%s\n",thiz->buffer);
	return RET_OK;
}

Ret   etk_text_buffer_delete(EtkTextBuffer* thiz, e32 offset, e32 length){
	e32 i = 0;
	char* dst = NULL;
	char* src = NULL;
	if(!(thiz != NULL && thiz->buffer != NULL ))
		return RET_FAIL;
	if(!(offset <= thiz->length))
		return RET_FAIL;

	length = (offset + length) < thiz->length ? length : (thiz->length - offset);
	
	dst = thiz->buffer + offset;
	src = thiz->buffer + offset + length;

	i = thiz->length - length - offset;
	for(; i != 0; i--, dst++, src++){
		*dst = *src;
	}
	thiz->length -= length;
	thiz->buffer[thiz->length] = '\0';

	return RET_OK;
}
Ret   etk_text_buffer_delete_chars(EtkTextBuffer* thiz, e32 offset, e32 count){
	e32 length=0;
	if(!((offset + count) <= (int)thiz->length && (offset + count) >= 0)){
		return RET_FAIL;
	}

	length =count;
	if(length == 0){
		return RET_FAIL;
	}

	if(length < 0){
		return etk_text_buffer_delete(thiz, offset+length, -length);
	}
	else{
		return etk_text_buffer_delete(thiz, offset, length);
	}
}
int  etk_text_buffer_char_bytes(EtkTextBuffer* thiz, e32 offset, int backward){
	return etk_text_buffer_chars_bytes(thiz, offset, backward ? -1 : 1);
}

//check fun to finish
int  etk_text_buffer_chars_bytes(EtkTextBuffer* thiz, e32 offset, e32 count){
	const char* iter = NULL;
	const char* offset_p = NULL;
	if(!(thiz != NULL && thiz->buffer != NULL))
		return 0;
	if(!(offset <= (int)thiz->length && offset >= 0))
		return 0;
	if((offset + count) > (int)thiz->length || (offset + count) <0){
		return 0;
	}
	offset_p = thiz->buffer + offset;
	iter = offset_p;
	if(count > 0){
		for(; count > 0; count--){
			 if(*iter == 0){
			 	break;
			 }	
			 iter++;
		}
	}
	else{
		for(; count < 0; count++){
			 if(*iter == 0){
			 	break;
			 }	
			 iter--;
		}

	}
	
	return iter - offset_p;
}
Ret etk_text_buffer_reset(EtkTextBuffer* thiz){
	return_val_if_fail(thiz != NULL, RET_FAIL);
	thiz->length = 0;

	return RET_OK;
}
char* etk_text_buffer_append_string(EtkTextBuffer* thiz, const char* str){
	e32 str_len = 0;
	char*  start = NULL;
	return_val_if_fail(thiz != NULL && str != NULL, NULL);
	
	str_len = strlen(str) + 1;
	return_val_if_fail(etk_text_buffer_extend(thiz, str_len) == RET_OK, NULL);

	start = thiz->buffer + thiz->length;
	strncpy(start, str, str_len);

	thiz->length += str_len;

	return start;
}
void  etk_text_buffer_destroy(EtkTextBuffer* thiz){
	if(thiz != NULL){
		ETK_FREE(thiz->buffer);
		ETK_ZFREE(thiz, sizeof(*thiz));
	}

}