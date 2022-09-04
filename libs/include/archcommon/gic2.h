/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef GIC2_H
#define GIC2_H

#include "libs/include/types.h"

//GIC v2 版本专用 gic400

#ifdef __cplusplus
#define __I volatile  ///< defines 'read only' permissions
#else
#define __I volatile const  ///< defines 'read only' permissions
#endif
#define __O volatile   ///< defines 'write only' permissions
#define __IO volatile  ///< defines 'read / write' permissions

//GIC v2手册 4.1.2 Distributor register map
typedef struct gic_dist {
  __IO uint32_t ctl;  ///< 0x000 Distributor Control Register
  __I uint32_t type;  ///< 0x004 Interrupt Controller Type Register
  __I uint32_t iid;   ///< 0x008 Distributor Implementer Identification Register
  __IO uint32_t RES1[5];       ///< 0x00C-0x01C Reserved
  __IO uint32_t IMP1[8];       ///< 0x020-0x03C IMPLEMENTATION DEFINED registers
  __IO uint32_t RES2[16];      ///< 0x040-0x07C Reserved
  __IO uint32_t igroup[16];    ///< 0x080 Interrupt Group Registers
  __IO uint32_t RES3[16];      ///< 0x080-0x0FC Reserved
  __IO uint32_t isenable[16];  ///< 0x100 Interrupt Set-Enable Registers
  __IO uint32_t RES4[16];      ///< Reserved
  __IO uint32_t icenable[16];  ///< 0x180 Interrupt Clear-Enable Registers
  __IO uint32_t RES5[16];      ///< Reserved
  __IO uint32_t ispend[16];    ///< 0x200 Interrupt Set-Pending Registers
  __IO uint32_t RES6[16];      ///< Reserved
  __IO uint32_t icpend[16];    ///< 0x280 Interrupt Clear-Pending Registers
  __IO uint32_t RES7[16];      ///< Reserved
  __IO uint32_t isactive[16];  ///< 0x300 GICv2 Interrupt Set-Active Registers
  __IO uint32_t RES8[16];      ///< Reserved
  __IO uint32_t icactive[16];  ///< 0x380 Interrupt Clear-Active Registers
  __IO uint32_t RES9[16];      ///< Reserved
  __IO uint32_t ipriority[128];  ///< 0x400 Interrupt Priority Registers
  __IO uint32_t RES10[128];      ///< Reserved
  __IO uint32_t itargets[128];  ///< 0x800 Interrupt Processor Targets Registers
  __IO uint32_t RES11[128];     ///< Reserved
  __IO uint32_t icfg[32];       ///< 0xC00 Interrupt Configuration Registers
  __IO uint32_t RES12[96];      ///< Reserved
  __IO uint32_t
      nsac[64];      ///< 0xE00 Non-secure Access Control Registers, optional
  __O uint32_t sgi;  ///< 0xF00 Software Generated Interrupt Register
  __IO uint32_t RES13[3];     ///< Reserved
  __IO uint32_t cpendsgi[4];  ///< 0xF10 SGI Clear-Pending Registers
  __IO uint32_t spendsgi[4];  ///< 0xF20 SGI Set-Pending Registers
  __IO uint32_t RES14[40];    ///< Reserved
  __I uint32_t PID4;          ///< Peripheral ID 4 Register
  __I uint32_t PID5;          ///< Peripheral ID 5 Register
  __I uint32_t PID6;          ///< Peripheral ID 6 Register
  __I uint32_t PID7;          ///< Peripheral ID 7 Register
  __I uint32_t PID0;          ///< Peripheral ID 0 Register
  __I uint32_t PID1;          ///< Peripheral ID 1 Register
  __I uint32_t PID2;          ///< Peripheral ID 2 Register [7:4] 0x2 for GICv2
  __I uint32_t PID3;          ///< Peripheral ID 3 Register
  __I uint32_t
      iccid[4];  ///< 0xFF0 Component ID Registers -> 0x00, 0xF0, 0x05, 0xB1
  __I uint32_t
      icpid[2];  ///< 0xFF0 Peripheral ID Registers -> 0x90, 0xB4 -> ARM GICv2
}gic_dist_t;

//GIC v2 手册4.1.3 CPU interface register map
typedef struct gic_cpu {
  volatile unsigned int ctl;        //0x0000 CPU Interface Control Register
  volatile unsigned int pm;         //0x0004 Interrupt Priority Mask Register
  volatile unsigned int bp;         //0x0008 Binary Point Registe
  volatile const unsigned int ia;   //0x000C Interrupt Acknowledge Register
  volatile unsigned int eoi;        //0x0010 End of Interrupt Register
  volatile const unsigned int rp;   //0x0014 Running Priority Register
  volatile const unsigned int hppi; //0x0018 Highest Priority Pending Interrupt Register
  volatile unsigned int abp;        //0x001C Aliased Binary Point Register
  volatile const unsigned int aia;  //0x0020 Aliased Interrupt Acknowledge Register
  volatile unsigned int aeoi;       //0x0024 Aliased End of Interrupt Register
  volatile const unsigned int ahppi;//0x0028 Aliased Highest Priority Pending Interrupt Register
  volatile unsigned int _res1[41];
  volatile unsigned int ap;         //0x00D0-0x00DC Active Priorities Registers
  volatile unsigned int _res2[3];
  volatile unsigned int nasp;       //0x00E0-0x00EC Non-secure Active Priorities Registers
  volatile unsigned int _res3[6];
  volatile const unsigned int iid;  //0x00FC CPU Interface Identification Register
  volatile unsigned int _res4[960];
  volatile unsigned int dir;        //0x1000 Deactivate Interrupt Register
}gic_cpu_t;



typedef struct gic{
    gic_dist_t* dist;
    gic_cpu_t*  cpu;
}gic_t;


void gic_init_base(void *cpu_addr, void *dist_addr);
void gic_enable(int cpu, int irq);
void gic_init();
void gic_send_sgi(int cpu, int irq);


#endif