/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef X86_MM_H
#define X86_MM_H

#define PAGE_P   1
#define PAGE_RWR   0 //读执行
#define PAGE_RWW   2 //读/写/执行
#define PAGE_USS   0 //系统级
#define PAGE_USU   4 //用户级
typedef u64* page_dir_t;

#endif