#include "etk_entry.h"
#include "etk_text_buffer.h"
#include "etk_platform.h"
#include "etk_wnd_manager.h"
#include "etk_input_method_manager.h"
#include "etk_key.h"

typedef struct PrivInfo_t{
	e32 caret;
	e32 visible_start;
	e32 visible_end;
	e32 selected_start;
	e32 selected_end;
	e32 input_type;
	e32 caret_visible;
	EtkPoint caret_pos;
	EtkTextBuffer *text_buffer;
}PrivInfo;



Ret etk_entry_move_caret(EtkWidget* thiz, int offset){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	priv->caret+=offset;
	if(priv->caret<0){
		priv->caret=0;
	}
}
Ret etk_entry_set_text_internal(EtkWidget* thiz, const char* text){
	PrivInfo *priv;
	return_val_if_fail(thiz != NULL && text != NULL, RET_FAIL);
	priv=(PrivInfo *)thiz->subclass;
	etk_text_buffer_delete(priv->text_buffer, 0, 10);
	etk_text_buffer_insert(priv->text_buffer, 0, text, -1);

	priv->visible_start =0;
	priv->visible_end = strlen(text);
	priv->caret = priv->visible_end;

}

const char* etk_entry_get_text(EtkWidget* thiz){
	return etk_widget_get_text(thiz);
}
Ret etk_entry_set_text(EtkWidget* thiz, const char* text){
	etk_widget_set_text(thiz,text);
}
Ret etk_entry_insert_text(EtkWidget* thiz,const char* text){
	PrivInfo *priv=(PrivInfo *)thiz->subclass;
	return_val_if_fail(thiz != NULL && text != NULL, RET_FAIL);

	if(priv->caret<(thiz->rect.width)>>3){
		etk_text_buffer_insert(priv->text_buffer, priv->caret, text, -1);
		priv->caret+=strlen(text);
		priv->visible_end=priv->caret;

	}else{
		etk_text_buffer_insert(priv->text_buffer,priv->visible_end, text, -1);
		priv->visible_start+=strlen(text);
		priv->visible_end+=strlen(text);
	}
	return RET_OK;
}

Ret etk_entry_insert_char(EtkWidget* thiz, char c){
	 char cc[2];
	 cc[0]=c;
	 cc[1]='\0';
	 return etk_entry_insert_text(thiz,cc);
}

Ret etk_entry_delete_text(EtkWidget* thiz,e32 length){
	PrivInfo *priv=(PrivInfo *)thiz->subclass;
	return_val_if_fail(thiz != NULL, RET_FAIL);
	//printf("caret:%d",priv->caret);
	if(priv->caret<(thiz->rect.width)>>3){
		if(priv->caret>=0)
			priv->caret-=length;
		priv->visible_end-=length;
		etk_text_buffer_delete(priv->text_buffer, priv->caret,length);
	}else{

		priv->visible_start-=length;
		priv->visible_end-=length;
		etk_text_buffer_delete(priv->text_buffer,priv->visible_end,length);
	}
	return RET_OK;
}



Ret etk_entry_on_paint_caret(EtkWidget* thiz){
	PrivInfo *priv;
	e32 count;
	priv=(PrivInfo *)thiz->subclass;
	if(thiz->type&ETK_WIDGET_SHARE_PARENT_CAVANS){
	// if is focus
		etk_canvas_vline(thiz->canvas,priv->caret<<3,4,thiz->rect.height-8,RED);
	}else{
		etk_canvas_vline(thiz->canvas,priv->caret<<3,4,thiz->rect.height+14,RED);
	}
	//etk_canvas_draw_rect(thiz->canvas,priv->caret<<3,0,2,thiz->rect.height-4,RED);
	//printf("c:%d\n",priv->caret);

}

 Ret etk_entry_on_paint(EtkWidget* thiz){
	PrivInfo *priv;
	e32 count;
	e8 *text;
	EtkBitmap* h;
	EtkRect r;
	return_val_if_fail(thiz != NULL, RET_FAIL);
	priv=(PrivInfo *)thiz->subclass;
	h=thiz->canvas->bitmap;
	r=thiz->rect;
	count=priv->visible_end-priv->visible_start;
	if(count>0){
		text=(e8 *)ETK_MALLOC(count+1);
		text=strncpy(text,priv->text_buffer->buffer+priv->visible_start,count);
		text[count]='\0';
		etk_widget_set_text(thiz,text);
		//printf("count:%d text:%s\n",count,etk_widget_get_text(thiz));
		ETK_FREE(text);
	}
	etk_canvas_fill_rect(thiz->canvas,0,0,thiz->rect.width,thiz->rect.height,thiz->canvas->gc.bgcolor.color);
	text=(e8 *)etk_widget_get_text(thiz);
	if(text!=NULL){
		//printf("text:%s\n",text);
		etk_canvas_draw_string(thiz->canvas,3,2,text);
	}
	
	if(thiz->type&ETK_WIDGET_SHARE_PARENT_CAVANS){
		etk_bitmap_hline(h,r.x,r.y,r.width-1,DIMGRAY);
		etk_bitmap_vline(h,r.x,r.y,r.height-1,DIMGRAY);

		etk_bitmap_hline(h,r.x,r.y+r.height-1,r.y+r.width-1,WHITESMOKE);
		etk_bitmap_vline(h,r.x+r.width-1,r.y+1,r.height-2,WHITESMOKE);

		etk_bitmap_hline(h,r.x+1,r.y+1,r.width-3,BLACK);
		etk_bitmap_vline(h,r.x+1,r.y+2,r.height-4,BLACK);

		etk_bitmap_hline(h,r.x+1,r.y+r.height-2,r.width-3,LIGHTGRAY);
		etk_bitmap_vline(h,r.y+r.width-2,r.y+2,r.height-4,LIGHTGRAY);
	}else{
		//etk_bitmap_draw_rect(h,1,1,r.width-2,r.height-2,thiz->canvas->gc.fgcolor.color);
		etk_bitmap_hline(h,0,0,r.width-1,DIMGRAY);
		etk_bitmap_vline(h,0,0,r.height-1,DIMGRAY);

		etk_bitmap_hline(h,0,r.height-1,r.width-1,WHITESMOKE);
		etk_bitmap_vline(h,r.width-1,1,r.height-2,WHITESMOKE);

		etk_bitmap_hline(h,1,1,r.width-3,BLACK);
		etk_bitmap_vline(h,1,2,r.height-4,BLACK);

		etk_bitmap_hline(h,1,r.height-2,r.width-3,LIGHTGRAY);
		etk_bitmap_vline(h,r.width-2,2,r.height-4,LIGHTGRAY);

	}

 }


Ret etk_entry_handle_key_event(EtkWidget* thiz, EtkEvent* event){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	Ret ret = RET_OK;
	

	switch(event->u.key.code){
		case ETK_KEY_CHOOSE_IME:
			etk_input_method_manager_focus_out(etk_default_input_method_manager(), thiz);
			//etk_input_method_chooser();
			etk_input_method_manager_focus_in(etk_default_input_method_manager(), thiz);
			break;
		case ETK_KEY_HOME:{
			etk_entry_move_caret(thiz,thiz->rect.width>>8-priv->caret);
			break;
		}
		case ETK_KEY_END:
			etk_entry_move_caret(thiz, thiz->rect.width>>8-priv->caret);
			break;
		case ETK_KEY_LEFT:
			//etk_entry_move_caret(thiz, -1);
			ret = RET_REMOVE;
			break;
		case ETK_KEY_RIGHT:
			//etk_entry_move_caret(thiz, 1);
			ret = RET_REMOVE;
			break;
		case ETK_KEY_UP:
		case ETK_KEY_DOWN:break;
		case ETK_KEY_DELETE:
		
			//etk_text_buffer_delete_chars(priv->text_buffer, priv->caret, 1);
			break;
		case ETK_KEY_BACKSPACE:{
			if(etk_entry_delete_text(thiz,1) == RET_OK){
				etk_canvas_clear_rect(thiz->canvas,0,0,thiz->rect.width,thiz->rect.height);
				etk_entry_on_paint(thiz);
				etk_entry_on_paint_caret(thiz);
				etk_widget_update_rect(thiz,&thiz->rect);
			}
			break;
		}
		default:{
			if(event->u.key.code < 0xff ){
				if(RET_OK==etk_entry_insert_char(thiz, event->u.key.code)){
					etk_canvas_clear_rect(thiz->canvas,0,0,thiz->rect.width,thiz->rect.height);
					etk_entry_on_paint(thiz);
					etk_entry_on_paint_caret(thiz);
					etk_widget_update_rect(thiz,&thiz->rect);
				}
			}
			break;
		}
	}

	return ret;

}



Ret etk_entry_on_event(EtkWidget* thiz, EtkEvent* event){
	EtkEvent e;
	Ret ret;
	PrivInfo *priv;
	return_val_if_fail(thiz != NULL && event != NULL, RET_FAIL);
	priv=(PrivInfo *)thiz->subclass;
	//printf("entry on event\n");
	ret=RET_OK;
	switch(event->type){
	case ETK_EVENT_SHOW:
		etk_entry_on_paint(thiz);
		etk_widget_paint(thiz);
		etk_event_init(&e, ETK_EVENT_SHOW);
		e.widget = thiz;
		etk_wnd_manager_dispatch_event(etk_default_wnd_manager(),&e);
		break;
	case ETK_EVENT_HIDE:

		break;
	case ETK_EVENT_FOCUS_IN:
		//printf("ETK_EVENT_FOCUS_IN\n");
		//to set input method
		etk_input_method_manager_focus_in(etk_default_input_method_manager(),thiz);
		//add timer source to source m
		
		break;
	case ETK_EVENT_FOCUS_OUT:
		etk_input_method_manager_focus_out(etk_default_input_method_manager(),thiz);
		//remove timer source or etc.

		break;
	case ETK_EVENT_KEY_DOWN:
		//printf("key douwn\n");
		ret = etk_entry_handle_key_event(thiz, event);
		break;
	case ETK_EVENT_KEY_UP:
	
		break;
	case ETK_EVENT_MOUSE_UP:
		//printf("mouse up\n");
		etk_event_init(&e, ETK_EVENT_FOCUS_IN);
		e.widget = thiz;
		etk_wnd_manager_dispatch_event(etk_default_wnd_manager(), &e);
		
		break;
	case ETK_EVENT_MOUSE_DOWN:
		/*printf("etk_entry_insert_text\n");
		etk_canvas_clear_rect(thiz->canvas,0,0,thiz->rect.width,thiz->rect.height);
		etk_entry_insert_text(thiz,bf);
		etk_entry_on_paint(thiz);
		etk_entry_on_paint_caret(thiz);
		bf[0]++;
		bf[1]='\0';*/
		
		//printf("%s",etk_entry_get_text(thiz));
		//etk_entry_insert_text(thiz,"ab");
	
		break;
	case ETK_EVENT_SET_TEXT:
		etk_entry_set_text_internal(thiz, (const char*)event->u.extra);
		//printf("set text:%s\n",(const char*)event->u.extra);
		etk_entry_on_paint(thiz);
		break;
	case ETK_EVENT_GET_TEXT:
		event->u.extra = (void*)etk_entry_get_text(thiz);
		break;
	}
	return ret;
}


 void etk_entry_destroy(EtkWidget* thiz){
	if(thiz != NULL){
		PrivInfo * priv=(PrivInfo *)thiz->subclass;
	/*	if(etk_widget_is_focused(thiz)){
			etk_input_method_manager_focus_out(etk_default_input_method_manager(), thiz);
		}

		ETK_FREE(priv->tips);
		etk_source_disable(priv->caret_timer);
		etk_main_loop_remove_source(etk_default_main_loop(), priv->caret_timer);
		etk_source_unref(priv->caret_timer);*/
		etk_text_buffer_destroy(priv->text_buffer);
		ETK_FREE(priv);
	}

	return;
}



EtkWidget* etk_entry_create(EtkWidget* parent, e32 x, e32 y, e32 width, e32 height){
	EtkWidget * thiz=(EtkWidget *)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo* priv;
	return_val_if_fail(thiz != NULL, NULL);
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	priv=(PrivInfo*)thiz->subclass;
	if(priv!=NULL){
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_ENTRY,0);
		thiz->event=etk_entry_on_event;
		thiz->paint=etk_entry_on_paint;
		thiz->destroy=etk_entry_destroy;
		thiz->rect.x+=parent->rect.x;
		thiz->rect.y+=parent->rect.y;
		etk_widget_append_child(parent, thiz);
		priv->text_buffer=etk_text_buffer_create(10);
		priv->caret=0;
		priv->selected_start=0;
		priv->selected_end=0;
		priv->visible_start=0;
		priv->visible_end=0;
		priv->caret_visible=0;
		priv->caret_pos.x=0;
		priv->caret_pos.y=0;


	}
	return thiz;
}
EtkWidget* etk_entry_create_ex(e32 x, e32 y, e32 width, e32 height,e32 attrib){
	EtkWidget * thiz=(EtkWidget *)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo* priv;
	return_val_if_fail(thiz != NULL, NULL);
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	priv=(PrivInfo*)thiz->subclass;
	if(priv!=NULL){
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_ENTRY|attrib,0);
		thiz->event=etk_entry_on_event;
		thiz->paint=etk_entry_on_paint;
		thiz->destroy=etk_entry_destroy;
		priv->text_buffer=etk_text_buffer_create(10);
		priv->caret=0;
		priv->selected_start=0;
		priv->selected_end=0;
		priv->visible_start=0;
		priv->visible_end=0;
		priv->caret_visible=0;
		priv->caret_pos.x=0;
		priv->caret_pos.y=0;


	}
	return thiz;
}
