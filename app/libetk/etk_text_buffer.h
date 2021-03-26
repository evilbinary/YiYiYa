#ifndef _ETK_TEXT_BUFFER_H
#define _ETK_TEXT_BUFFER_H

#include "etk_type.h"

typedef struct EtkTextBuffer_t{
	char* buffer;
	e32 length;
	e32 buffer_length;
}EtkTextBuffer;


EtkTextBuffer* etk_text_buffer_create(e32 init_length);
Ret   etk_text_buffer_insert(EtkTextBuffer* thiz, e32 offset, const char* text, e32 len);
Ret   etk_text_buffer_delete(EtkTextBuffer* thiz, e32 offset, e32 length);
Ret   etk_text_buffer_delete_chars(EtkTextBuffer* thiz, e32 offset, e32 count);
int   etk_text_buffer_chars_bytes(EtkTextBuffer* thiz, e32 offset, e32 count);
Ret   etk_text_buffer_reset(EtkTextBuffer* thiz);
char* etk_text_buffer_append_string(EtkTextBuffer* thiz, const char* str);
void  etk_text_buffer_destroy(EtkTextBuffer* thiz);


#endif