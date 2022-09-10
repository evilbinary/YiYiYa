#ifndef __V3S_REG_DE_H__
#define __V3S_REG_DE_H__

#include "libs/include/types.h"

#define V3S_DE_BASE			(0x01000000)

#define V3S_DE_MUX_GLB		(0x00100000 + 0x00000)
#define V3S_DE_MUX_BLD		(0x00100000 + 0x01000)
#define V3S_DE_MUX_CHAN		(0x00100000 + 0x02000)
#define V3S_DE_MUX_VSU		(0x00100000 + 0x20000)
#define V3S_DE_MUX_GSU1		(0x00100000 + 0x30000)
#define V3S_DE_MUX_GSU2		(0x00100000 + 0x40000)
#define V3S_DE_MUX_GSU3		(0x00100000 + 0x50000)
#define V3S_DE_MUX_FCE		(0x00100000 + 0xa0000)
#define V3S_DE_MUX_BWS		(0x00100000 + 0xa2000)
#define V3S_DE_MUX_LTI		(0x00100000 + 0xa4000)
#define V3S_DE_MUX_PEAK		(0x00100000 + 0xa6000)
#define V3S_DE_MUX_ASE		(0x00100000 + 0xa8000)
#define V3S_DE_MUX_FCC		(0x00100000 + 0xaa000)
#define V3S_DE_MUX_DCSC		(0x00100000 + 0xb0000)

struct de_clk_t {
	u32 gate_cfg;
	u32 bus_cfg;
	u32 rst_cfg;
	u32 div_cfg;
	u32 sel_cfg;
};

struct de_glb_t {
	u32 ctl;
	u32 status;
	u32 dbuff;
	u32 size;
};

struct de_bld_t {
	u32 fcolor_ctl;
	struct {
		u32 fcolor;
		u32 insize;
		u32 offset;
		u32 dum;
	} attr[4];
	u32 dum0[15];
	u32 route;
	u32 premultiply;
	u32 bkcolor;
	u32 output_size;
	u32 bld_mode[4];
	u32 dum1[4];
	u32 ck_ctl;
	u32 ck_cfg;
	u32 dum2[2];
	u32 ck_max[4];
	u32 dum3[4];
	u32 ck_min[4];
	u32 dum4[3];
	u32 out_ctl;
};

struct de_vi_t {
	struct {
		u32 attr;
		u32 size;
		u32 coord;
		u32 pitch[3];
		u32 top_laddr[3];
		u32 bot_laddr[3];
	} cfg[4];
	u32 fcolor[4];
	u32 top_haddr[3];
	u32 bot_haddr[3];
	u32 ovl_size[2];
	u32 hori[2];
	u32 vert[2];
};

struct de_ui_t {
	struct {
		u32 attr;
		u32 size;
		u32 coord;
		u32 pitch;
		u32 top_laddr;
		u32 bot_laddr;
		u32 fcolor;
		u32 dum;
	} cfg[4];
	u32 top_haddr;
	u32 bot_haddr;
	u32 ovl_size;
};

#endif /* __V3S_REG_DE_H__ */
