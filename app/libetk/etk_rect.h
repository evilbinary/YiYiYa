#ifndef _ETK_RECT_H
#define _ETK_RECT_H

#include "etk_type.h"


struct EtkRect_t;
typedef struct EtkRect_t EtkRect;

struct EtkRect_t{
	e32 x,y;
	e32 width;
	e32 height;
	struct EtkRect_t *next;
};

struct EtkRegion_t;
typedef struct EtkRegion_t EtkRegion;

struct EtkRegion_t{
	struct EtkRect_t *outputs;
	struct EtkRect_t *outputs_tail;
};

EtkRect etk_rect_min(EtkRect r0,EtkRect r1);

e32 is_rect_cross(EtkRect r0, EtkRect r1);
EtkRect *rect_intersect(EtkRect r0, EtkRect r1);
EtkRect etk_rect_intersect(EtkRect r0, EtkRect r1);
EtkRect * get_rect(EtkRect *rects, e32 index);
EtkRegion * get_region(EtkRegion *region, e32 index);
EtkPoint *etk_line_intersect(e32 x1, e32 x2, e32 x3, e32 x4);
e32 etk_line_intersect1(e32 *rx,e32 *ry,e32 x1, e32 x2, e32 x3, e32 x4);

EtkRect * etk_rect_sub(EtkRect *rect, EtkRect *cliprect);
EtkRegion *etk_region_clip_rect(EtkRegion *region,EtkRect *cliprect);

EtkRect * etk_rect_create(e32 x, e32 y, e32 width, e32 height);
EtkRect *etk_rect_remove(EtkRect* rects,EtkRect *rect);
EtkRect *etk_rect_get_equal(EtkRect* rects,EtkRect *rect);
EtkRect *etk_rect_copy(EtkRect *rect);
EtkRect * etk_rect_remove_all(EtkRect *rects);
void etk_rect_set(EtkRect *rect,e32 x,e32 y,e32 width,e32 height);
EtkRect * etk_rect_allocate();
void etk_rect_revoke(EtkRect *p);

#define  etk_rect_add_tail(head,tail,rect) if (head == NULL) {head = rect;tail = rect;} else {tail->next = rect;tail = rect;}
EtkRegion * etk_region_create();


#endif