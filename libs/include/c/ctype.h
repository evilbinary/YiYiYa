#ifndef _CTYPE_H
#define _CTYPE_H


#ifdef __cplusplus
extern "C" {
#endif

int   isalnum(int);
int   isalpha(int);
int   isascii(int);
int   iscntrl(int);
int   isdigit(int);
int   isgraph(int);
int   islower(int);
int   isprint(int);
int   ispunct(int);
int   isspace(int);
int   isupper(int);
int   isxdigit(int);
int   toascii(int);
int   tolower(int);
int   toupper(int);



#define _toupper(i) toupper(i);
#define _tolower(i) tolower(i);

#ifdef __cplusplus
}
#endif


#endif


