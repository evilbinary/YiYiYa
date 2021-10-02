/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef ARM_MM_H
#define ARM_MM_H


#define L1_PAGE_TABLE (1 << 0)
#define L2_SMALL_PAGE (2 << 0)

#define L1_PXN (0 << 2)  // The Privileged execute-never bit
#define L1_NS (0 << 3)   // Non-secure bit
#define L1_SBZ (0 << 4)  // Should be Zero

#define L1_DOMAIN(n) (n << 5)

#define L2_XN (0 << 0)  // The Execute-never bit
#define L2_CB (3 << 2)  // 0b11 cache write-back
#define L2_NCNB (0 << 2) // 0b00 Non-cacheable
#define L2_NCB (1 << 2) // 0b01 Write-Back, Write-Allocate
#define L2_CNB (0 << 2) // 0b10 Write-Through, no Write-Allocate


#define L2_AP_ACCESS (3 << 4)
#define L2_AP_RW (0 << 9)  // read write any privilege level
#define L2_TEXT (7 << 6)

#define L2_TEXT_0 (0 << 6)
#define L2_TEXT_1 (1 << 6)
#define L2_TEXT_2 (2 << 6)


#define L2_S (1 << 10)   // The Shareable bit
#define L2_NG (1 << 11)  // The not global bit
#define L2_G (0 << 11)  // The translation is global, meaning the region is available for all processes.


#define L1_DESC (L1_PAGE_TABLE | L1_PXN | L1_NS | L1_SBZ | L1_DOMAIN(0))
#define L2_DESC \
  (L2_XN | L2_SMALL_PAGE | L2_NCNB | L2_AP_ACCESS | L2_AP_RW | L2_S | L2_G)

#define PAGE_P   0
#define PAGE_RWR   0 //读执行
#define PAGE_RWW   0 //读/写/执行
#define PAGE_USS   (L2_TEXT_1|L2_CB) //系统级
#define PAGE_USU   (L2_TEXT_1|L2_CB) //用户级
#define PAGE_DEV   (L2_TEXT_0|L2_NCNB) //设备级

typedef u32 page_dir_t;

#endif