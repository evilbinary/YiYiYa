#include "etk_rect.h"
#include "etk_widget.h"
#include "etk_stack.h"

STACK_TEMPLATE(EtkStack,void*);

EtkRect *cliptail=NULL;

EtkRect etk_rect_min(EtkRect r0,EtkRect r1){
	EtkRect r;
	r.x=ETK_MIN(r0.x,r1.x);
	r.y=ETK_MIN(r0.y,r1.y);
	r.width=ETK_MAX((r0.x+r0.width),(r1.x+r1.width));
	r.height=ETK_MAX((r0.y+r0.height),(r1.y+r1.height));
	r.width-=r.x;
	r.height-=r.y;
	return r;
}

e32 is_rect_cross(EtkRect r0, EtkRect r1) {
	e32 r0_left = r0.x;
	e32 r0_right = r0.x + r0.width;
	e32 r0_top = r0.y;
	e32 r0_bottom = r0.y + r0.height;

	e32 r1_left = r1.x;
	e32 r1_right = r1.x + r1.width;
	e32 r1_top = r1.y;
	e32 r1_bottom = r1.y + r1.height;
	return !(r0_left > r1_right || r0_right < r1_left || r0_bottom < r1_top || r0_top > r1_bottom);
}
EtkRect *rect_intersect(EtkRect r0, EtkRect r1) {
	EtkRect *r = (EtkRect*)ETK_MALLOC(sizeof(EtkRect));
	e32 left, right, top, bottom;
	r->next = NULL;
	left = ETK_MAX(r0.x,r1.x);
	right = ETK_MIN(r0.x+r0.width,r1.x+r1.width);
	top = ETK_MAX(r0.y,r1.y);
	bottom = ETK_MIN(r0.y+r0.height,r1.y+r1.height);
	if (left < right && bottom > top) {
		r->x = left;
		r->y = top;
		r->width = right - left;
		r->height = bottom - top;
		return r;
	} else {
		r->x = -1;
		r->y = -1;
		r->height = -1;
		r->width = -1;
	}
	return r;
}
EtkRect etk_rect_intersect(EtkRect r0, EtkRect r1) {
	EtkRect r;
	e32 left, right, top, bottom;
	r.next = NULL;
	left = ETK_MAX(r0.x,r1.x);
	right = ETK_MIN(r0.x+r0.width,r1.x+r1.width);
	top = ETK_MAX(r0.y,r1.y);
	bottom = ETK_MIN(r0.y+r0.height,r1.y+r1.height);
	if (left < right && bottom > top) {
		r.x = left;
		r.y = top;
		r.width = right - left;
		r.height = bottom - top;
		return r;
	} else {
		r.x = -1;
		r.y = -1;
		r.height = -1;
		r.width = -1;
	}
	return r;
}
EtkRect * get_rect(EtkRect *rects, e32 index) {
	EtkRect *p = rects;
	e32 i = 0;
	while (p != NULL) {
		if (index == i)
			return p;
		i++;
		p = p->next;
	}
	return NULL;
}



inline EtkPoint *etk_line_intersect(e32 x1, e32 x2, e32 x3, e32 x4) {
	e32 max = ETK_MAX(x1,x3);
	e32 min = ETK_MIN(x2,x4);
	//printf("x1:%d x2:%d x3:%d x4:%d\n",x1,x2,x3,x4);
	if (max <= min) {
		EtkPoint *p = (EtkPoint *) ETK_MALLOC(sizeof(EtkPoint));
		p->x = max;
		p->y = min;
		//printf("done\n");
		return p;
	} else {
		return NULL;
	}
}

inline e32 etk_line_intersect1(e32 *rx,e32 *ry,e32 x1, e32 x2, e32 x3, e32 x4) {
	e32 max = ETK_MAX(x1,x3);
	e32 min = ETK_MIN(x2,x4);
	if (max <= min) {
		*rx = max;
		*ry = min;
		//printf("x1:%d x2:%d x3:%d x4:%d",x1,x2,x3,x4);
		//printf(" max(%d,%d)=%d min(%d,%d)=%d\n",x1,x3,max,x2,x4,min);
		//draw_line(p.x,100,p.y,100);
		return 1;
	} else {
		return 0;
	}

	
}

EtkWidget *etk_widget_clip_rect(EtkWidget *widget,EtkRect *rect){
	EtkWidget *w=widget;
	EtkWidget *w1=NULL;
	EtkStack s,s1;
	struct STACK_NODE(EtkStack) *p=NULL;
	struct STACK_NODE(EtkStack) *p1=NULL;
	STACK_INIT(&s);
	STACK_INIT(&s1);
	while(true){
		//visit w
		if(w==NULL){
			break;
		}
		if(w->visable==1){
			w1=widget;
			//draw_rect(w->rect,RED);
			//printf("(%d,%d,%d,%d)\n",w->rect.x,w->rect.y,w->rect.width,w->rect.height);
			while(true){
				//visit w1
				EtkRect *inter;
				if(w1==NULL){
					break;
				}
				//if(w1->visable==1){
					inter=rect_intersect(w->rect,*rect);
					w->canvas->region->outputs=etk_rect_remove_all(w->canvas->region->outputs);
					if(w->canvas->region->outputs==NULL){
						w->canvas->region->outputs=etk_rect_copy(inter);
						w->canvas->region->outputs_tail=w->canvas->region->outputs;		
					}
					if(inter->width>0){
						//printf("vv:%d\n",w1->visable);
						if(w!=w1&&(w1->visable==1)){	//if(w==w1){break;}
							w1->canvas->region=etk_region_clip_rect(w1->canvas->region,inter);
						}
					}
					ETK_FREE(inter);
					if(w1->next!=NULL){
						
							STACK_NEW_NODE(EtkStack,p1,w1->next);
							STACK_PUSH(&s1,p1);
					}
					w1=w1->chil;
					if( w1 ==NULL){
						if(STACK_IS_EMPTY(&s1)){
							break;
						}else{
							STACK_POP(&s1,p1);
							w1=(EtkWidget*)p1->data;
							ETK_FREE(p1);
						}
					}
				//}
			}
		}
		if(w->next!=NULL){
				STACK_NEW_NODE(EtkStack,p,w->next);
				STACK_PUSH(&s,p);
		}
		w=w->chil;
		if(w==NULL){
			if(STACK_IS_EMPTY(&s)){
				break;
			}else{
				STACK_POP(&s,p);
				w=(EtkWidget*)p->data;
				ETK_FREE(p);
			}
		}
	}
	/*if(STACK_IS_EMPTY(&s)){
	printf("is empty ");
	}
	if(STACK_IS_EMPTY(&s1)){
	printf("is empty1 ");
	}*/
	return widget;
}


EtkRegion *etk_region_clip_rect(EtkRegion *region,EtkRect *cliprect){
	EtkRect *p=region->outputs;
	EtkRect *new_outputs=NULL;
	EtkRect *new_outputs_tail=NULL;
	while(p!=NULL){
		EtkRect *head=NULL;
		head=etk_rect_sub(p,cliprect);
		if(head!=NULL){
			if(new_outputs==NULL){
				new_outputs=head;
				new_outputs_tail=cliptail;
			}else{
				new_outputs_tail->next=head;
				new_outputs_tail=cliptail;
			}
		}
		p=p->next;
	}
	region->outputs=etk_rect_remove_all(region->outputs);
	region->outputs=new_outputs;
	return region;
}

/*	2012-1-17
 *  ���μ��о���
 * ���أ����еĸ���
 * if return null mean cover all
 */
EtkRect * etk_rect_sub(EtkRect *rect, EtkRect *cliprect){

	e32 x1 = rect->x;
	e32 y1 = rect->y;
	e32 x2 = rect->x + rect->width;
	e32 y2 = rect->y + rect->height;
	e32 x3 = cliprect->x;
	e32 y3 = cliprect->y;
	e32 x4 = cliprect->x + cliprect->width;
	e32 y4 = cliprect->y + cliprect->height;
	//EtkPoint *k, *kk;
	e32 k,kk;
	EtkRect *head = NULL;
	EtkRect *tail = NULL;
	e32 k1, k2,k3, k4;
	e32 count=0;
	k=etk_line_intersect1(&k1,&k2,y1, y2, y3, y4);
	kk=etk_line_intersect1(&k3,&k4,x1, x2, x3, x4);
	if(k!=0&&kk!=0){
		//printf("cross\n");
		//ˮƽ�и�
		if (y1 < k1) {
			EtkRect *r = etk_rect_create(x1, y1, x2 - x1, k1 - y1);
			etk_rect_add_tail(head, tail, r);
			//count += 1;
			//			fill_rect_plus(*r, BLUE);
		}
		if (k2 < y2) { //valid is 1
			EtkRect *r = etk_rect_create(x1, k2, x2 - x1, y2 - k2);
			etk_rect_add_tail(head, tail, r);
			//count += 1;
		}
		//��ֱ�ָ�
		if (x1 < k3) {
			EtkRect *r = etk_rect_create(x1, k1, k3 - x1, k2 - k1);
			etk_rect_add_tail(head, tail, r);
			count += 1;
		}
		if (k4 < x2) {
			EtkRect *r = etk_rect_create(k4, k1, x2 - k4, k2 - k1);
			etk_rect_add_tail(head, tail, r);
			count += 1;
		}
		
	} else {
		//ȫ��δ������
		EtkRect *r = etk_rect_create(rect->x, rect->y, rect->width, rect->height);
		//		r->isvalid = 1;
		count += 1;
		etk_rect_add_tail(head, tail, r);
		//dbg_printf("not cover at all");
	}
	//if(head!=NULL){//should check this code to let work
	//	//dbg_printf("head is not null\n");
	//	/*head->prev = rect->prev;
	//	if (rect->prev != NULL){
	//		if(head->prev!=NULL)
	//			head->prev->next = head;
	//	}
	//	tail->next = rect->next;
	//	if (tail->next != NULL)
	//		tail->next->prev = tail;*/
	//	//draw_rects_minus(head,2, BLUE);
	//}else{
	//	//dbg_printf("head is null\n");
	//	//printf("count:%d\n",*count);
	//}
	cliptail=tail;
	return head;
}

EtkRect * etk_rect_create(e32 x, e32 y, e32 width, e32 height) {
	EtkRect *rect = (EtkRect*) ETK_MALLOC(sizeof(EtkRect)); //etk_rect_allocate(); //
	rect->x = x;
	rect->y = y;
	rect->width = width;
	rect->height = height;
	rect->next = NULL;
	return rect;
}
//return the result of unremove sets 
EtkRect *etk_rect_remove(EtkRect* head,EtkRect *rect){//uncheck
	EtkRect *new_head=head;
	EtkRect *p=head;
	if(head==NULL){
		new_head=NULL;
	}else{
		if(p==rect){
			new_head=p->next;
		}else{
			while(p->next!=NULL){
				if(p->next==rect){
					p->next=p->next->next;
					//printf("remov\n");
					break;
				}
				p=p->next;
			}
		}
	}
	return new_head;

}
EtkRect *etk_rect_remove_equal(EtkRect* rects,EtkRect *rect){
	return etk_rect_remove(rects,etk_rect_get_equal(rects,rect));
}
EtkRect *etk_rect_get_equal(EtkRect* rects,EtkRect *rect){
	EtkRect *p=rects;
	while(p!=NULL){
		if(p->x==rect->x&&p->y==rect->y&&p->width==rect->width&&p->height==rect->height){
			return p;
		}
		p=p->next;
	}
	return NULL;
}
EtkRect *etk_rect_copy(EtkRect *rect){
	return etk_rect_create(rect->x,rect->y,rect->width,rect->height);
}
EtkRect * etk_rect_remove_all(EtkRect *rects){
	EtkRect *t=NULL;
	while(rects!=NULL){
		t=rects;
		rects=rects->next;
		t->next=NULL;
		ETK_FREE(t);
		t=NULL;
	}
	return NULL;
}
void etk_rect_set(EtkRect *rect,e32 x,e32 y,e32 width,e32 height){
	rect->x=x;
	rect->y=y;
	rect->height=height;
	rect->width=width;
}

EtkRegion * etk_region_create() {
	EtkRegion *region = (EtkRegion *) ETK_MALLOC(sizeof(EtkRegion));
	region->outputs=NULL;
	region->outputs_tail=NULL;

	return region;
}


EtkRect * etk_rect_allocate() {
	return NULL;
}
void etk_rect_revoke(EtkRect *p) {

}
