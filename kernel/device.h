/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef DEVICE_H
#define DEVICE_H

#include "arch/arch.h"

#define DEVICE_TYPE_CHAR 1
#define DEVICE_TYPE_BLOCK 2
#define DEVICE_TYPE_VIRTUAL 3
#define MAX_DEVICE 256



typedef struct fd{
    u32 id;
    u32 type; // file ,socket, pipe dir
}fd_t;

typedef struct stat {
	size_t size;
	int is_dir;
}stat_t;

typedef size_t (*dev_read_t)(fd_t*fd, void *buf, size_t len);
typedef size_t (*dev_write_t)(fd_t*fd, const void *buf, size_t len);
typedef int (*dev_stat_t)(fd_t *fd, stat_t *stat);

typedef struct device{
    char* name;
    u32 id;
    u32 type; //char block 
	dev_read_t read;
    dev_write_t write;
    dev_stat_t stat;
}device_t;

void device_add(device_t* device);

void device_remove();

device_t* device_find(u32 id);


#endif