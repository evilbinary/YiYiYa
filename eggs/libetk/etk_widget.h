#ifndef _ETK_WIDGET_H
#define _ETK_WIDGET_H
#include "etk_display.h"
#include "etk_type.h"
#include "etk_canvas.h"
#include "etk_event.h"
#include "etk_rect.h"


#define ETK_WIDGET_DESKTOP				0x0001
#define ETK_WIDGET_CHILD				0x0002
#define ETK_WIDGET_SHARE_PARENT_CAVANS	0x0004
#define ETK_WIDGET_SHARE_CAVANS			0x0008
#define ETK_WIDGET_NO_CAVANS			0x0010
#define ETK_WIDGET_OWN_CAVANS			0x0020
#define ETK_WIDGET_NO_BITMAP			0x0040

#define ETK_WIDGET_BUTTON				0x0080
#define ETK_WIDGET_WINDOW				0x0100
#define ETK_WIDGET_MENU					0x0200
#define ETK_WIDGET_MENU_ITEM			0x0400
#define ETK_WIDGET_ENTRY				0x0800


#define ETK_WIDGET_DIALOG				0x1000
#define ETK_WIDGET_GROUP_BOX			0x2000
#define ETK_WIDGET_RADIO_BUTTON			0x4000
#define ETK_WIDGET_CHECK_BUTTON			0x8000
#define ETK_WIDGET_INPUT_METHOD			0x01000

#define etk_is_widget(type,widget_type) (type&(~widget_type)==widget_type)

struct EtkWidget_t;
struct EtkWidgetInfo_t;

typedef struct EtkWidget_t EtkWidget;
typedef struct EtkWidgetInfo_t EtkWidgetInfo;

typedef Ret  (*EtkWidgetPaint)(EtkWidget* thiz);
typedef Ret  (*EtkWidgetEvent)(EtkWidget* thiz, EtkEvent* event);
typedef void (*EtkWidgetDestroy)(EtkWidget* thiz);
struct EtkWidget_t{
	//property
	e32 type;	//��������
	EtkRect rect;		//��������
	EtkWidget *parent;	//���׹���
	EtkWidget *chil;	//���ӹ���
	EtkWidget *prev;	//ǰһ������
	EtkWidget *next;	//��һ������
	e32 visable;		//�ɼ�����
	//method
	EtkDisplay *display;	//��ʾ
	EtkCanvas *canvas;		//����
	EtkWidgetPaint paint;	//����
	EtkWidgetEvent event;	//�¼�
	EtkWidgetDestroy destroy;//����

	EtkWidgetInfo *info;	//��Ϣ
	void *subclass;			//����
};


void etk_widget_init(EtkWidget *thiz,e32 x,e32 y,e32 width,e32 height,e32 type,e32 attr);

const char * etk_widget_get_text(EtkWidget* thiz);
Ret etk_widget_set_text(EtkWidget* thiz,const char* text);

Ret etk_widget_paint_self(EtkWidget* thiz);
Ret etk_widget_paint(EtkWidget* thiz);
Ret etk_widget_paint_child(EtkWidget* thiz);
Ret etk_widget_paint_all(EtkWidget* thiz);
Ret etk_widget_update(EtkWidget* thiz);
Ret etk_widget_update_rect(EtkWidget* thiz,EtkRect *rect);

Ret etk_widget_event(EtkWidget* thiz, EtkEvent* event);
e32 etk_widget_is_visible(EtkWidget* thiz);
void etk_widget_show(EtkWidget* thiz, int visible);
void etk_widget_show_all(EtkWidget* thiz);

void etk_widget_append_next(EtkWidget* thiz, EtkWidget* next);
void etk_widget_append_child(EtkWidget* thiz, EtkWidget* child);
void etk_widget_add_child(EtkWidget* thiz, EtkWidget* child);
void etk_widget_append(EtkWidget* thiz, EtkWidget* widget);
void etk_widget_remove_child(EtkWidget* thiz,EtkWidget* child);

#endif