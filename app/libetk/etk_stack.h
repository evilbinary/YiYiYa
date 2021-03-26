#ifndef _ETK_STACK_H
#define _ETK_STACK_H

#include "etk_platform.h"

#define STACK_NODE(name) name##__node

#define STACK_TEMPLATE(name,type)             \
struct STACK_NODE(name)                       \
{                                             \
    void *data;                                \
    struct STACK_NODE(name) *next;            \
};                                            \
typedef struct                                \
{                                             \
    struct STACK_NODE(name) *top;             \
}name

#define STACK_INIT(stack)                     \
do{(stack)->top=NULL;}while(0)

#define STACK_IS_EMPTY(stack)                 \
((stack)->top==NULL)

#define STACK_GET_TOP(stack)                  \
((stack)->top->data)

#define STACK_POP(stack,node)                 \
do{                                           \
    if((stack)->top==NULL) break;             \
    (node)=(stack)->top;                      \
    (stack)->top=(stack)->top->next;          \
}while(0)

#define STACK_PUSH(stack,node)                \
do{                                           \
    if((node)==NULL) break;                   \
    (node)->next=(stack)->top;                \
    (stack)->top=(node);                      \
}while(0)

#define STACK_NEW_NODE(name,node,value)                    \
do{                                                        \
    (node) = (struct STACK_NODE(name)*)                    \
           ETK_MALLOC(sizeof(struct STACK_NODE(name)));        \
    if((node)) (node)->data = value;                       \
}while(0)

#define STACK_DEL_NODE(node)                               \
do{                                                        \
    free((node));                                          \
    (node) = NULL;                                         \
}while(0)


#endif