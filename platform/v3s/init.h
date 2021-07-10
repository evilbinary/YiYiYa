#ifndef INIT_H
#define INIT_H


#include "libs/include/types.h"

#define	CTL_ENABLE		0x01
#define	CTL_RELOAD		0x02		/* reload ival */
#define	CTL_SRC_32K		0x00
#define	CTL_SRC_24M		0x04

typedef struct v3s_timer {
	volatile unsigned int irq_ena;		/* 00 */
	volatile unsigned int irq_status;	/* 04 */
	int __pad1[2];
	volatile unsigned int t0_ctrl;		/* 10 */
	volatile unsigned int t0_ival;		/* 14 */
	volatile unsigned int t0_cval;		/* 18 */
	int __pad2;
	volatile unsigned int t1_ctrl;		/* 20 */
	volatile unsigned int t1_ival;		/* 24 */
	volatile unsigned int t1_cval;		/* 28 */
}v3s_timer_t;


#endif