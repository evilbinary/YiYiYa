/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef ARM_MM_H
#define ARM_MM_H

#define PAGE_P   0
#define PAGE_RWR   0 //读执行
#define PAGE_RWW   0 //读/写/执行
#define PAGE_USS    //系统级
#define PAGE_USU    //用户级
#define PAGE_DEV    //设备级

typedef u32 page_dir_t;

#endif