#ifndef __V3S_REG_TCON_H__
#define __V3S_REG_TCON_H__

#include "libs/include/types.h"


#define V3S_TCON_BASE			(0x01c0c000)

struct v3s_tcon_reg_t {
	u32 ctrl;					/* 0x00 */
	u32 int0;					/* 0x04 */
	u32 int1;					/* 0x08 */
	u8 res0[0x04];			/* 0x0c */
	u32 tcon0_frm_ctrl;		/* 0x10 */
	u32 tcon0_frm_seed[6];	/* 0x14 */
	u32 tcon0_frm_table[4];	/* 0x2c */
	u8 res1[4];				/* 0x3c */
	u32 tcon0_ctrl;			/* 0x40 */
	u32 tcon0_dclk;			/* 0x44 */
	u32 tcon0_timing_active;	/* 0x48 */
	u32 tcon0_timing_h;		/* 0x4c */
	u32 tcon0_timing_v;		/* 0x50 */
	u32 tcon0_timing_sync;	/* 0x54 */
	u32 tcon0_hv_intf;		/* 0x58 */
	u8 res2[0x04];			/* 0x5c */
	u32 tcon0_cpu_intf;		/* 0x60 */
	u32 tcon0_cpu_wr_dat;		/* 0x64 */
	u32 tcon0_cpu_rd_dat0;	/* 0x68 */
	u32 tcon0_cpu_rd_dat1;	/* 0x6c */
	u32 tcon0_ttl_timing0;	/* 0x70 */
	u32 tcon0_ttl_timing1;	/* 0x74 */
	u32 tcon0_ttl_timing2;	/* 0x78 */
	u32 tcon0_ttl_timing3;	/* 0x7c */
	u32 tcon0_ttl_timing4;	/* 0x80 */
	u32 tcon0_lvds_intf;		/* 0x84 */
	u32 tcon0_io_polarity;	/* 0x88 */
	u32 tcon0_io_tristate;	/* 0x8c */
	u32 tcon1_ctrl;			/* 0x90 */
	u32 tcon1_timing_source;	/* 0x94 */
	u32 tcon1_timing_scale;	/* 0x98 */
	u32 tcon1_timing_out;		/* 0x9c */
	u32 tcon1_timing_h;		/* 0xa0 */
	u32 tcon1_timing_v;		/* 0xa4 */
	u32 tcon1_timing_sync;	/* 0xa8 */
	u8 res3[0x44];			/* 0xac */
	u32 tcon1_io_polarity;	/* 0xf0 */
	u32 tcon1_io_tristate;	/* 0xf4 */
	u8 res4[0x108];			/* 0xf8 */
	u32 mux_ctrl;				/* 0x200 */
	u8 res5[0x1c];			/* 0x204 */
	u32 lvds_ana0;			/* 0x220 */
	u32 lvds_ana1;			/* 0x224 */
};

#endif /* __V3S_REG_TCON_H__ */
