#ifndef _ETK_WINDOW_H
#define _ETK_WINDOW_H

#include "etk_widget.h"

struct EtkWindowClass;
typedef struct EtkWindowClass_t EtkWindowClass;

struct EtkWindowClass_t{
	char *title;	//����
	e32 type;		//����
	e32 has_head;	//�Ƿ���ͷ
	EtkRect head_rect;//ͷ������
	EtkRect close_rect;//�ر�����
	EtkWidget *modal;	//ģ̬�ؼ�
	void *inner_child;	//�ڲ��ӿؼ�
	void *data[2];		//����
};

EtkWidget* etk_create_window(e32 x,e32 y,e32 width,e32 height,e32 type);
EtkWidget* etk_window_create(e32 x,e32 y,e32 width,e32 height,e32 type);

Bool etk_window_has_head(EtkWidget *thiz);
Ret etk_window_on_paint(EtkWidget* thiz);
Ret etk_window_on_paint_head(EtkWidget* thiz);
Ret etk_window_on_paint_head_with_color(EtkWidget* thiz,u32 fgcolor,u32 bgcolor);

Ret etk_window_on_event(EtkWidget* thiz, EtkEvent* event);
Ret etk_window_on_key_event(EtkWidget* thiz, EtkEvent* event);
Ret etk_window_on_mouse_event(EtkWidget* thiz, EtkEvent* event);

Ret etk_window_update(EtkWidget* thiz, EtkRect* rect);

EtkRect etk_window_get_head_rect(EtkWidget* thiz);
EtkRect etk_window_get_close_rect(EtkWidget* thiz);
void etk_window_append_child(EtkWidget* parent, EtkWidget* child);

void etk_window_show(EtkWidget* thiz);
void etk_window_show_all(EtkWidget* thiz);
void etk_window_hide_all(EtkWidget* thiz);
void etk_window_set_all_visible(EtkWidget *thiz,e32 visible);

void etk_window_destroy(EtkWidget *thiz);

#endif