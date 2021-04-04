/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef PTY_H
#define PTY_H

#include "kernel/kernel.h"
#include "algorithm/buffer.h"

#define PTY_BUFFER 128

#define IOC_PTY_MAGIC 'p'

#define IOC_SLAVE _IOW(IOC_PTY_MAGIC, 1, int)
#define IOC_READ_OFFSET _IOW(IOC_PTY_MAGIC, 3, int)
#define IOC_WRITE_OFFSET _IOW(IOC_PTY_MAGIC, 4, int)

typedef struct pty{
    buffer_t *in;
    buffer_t* out;

    vnode_t* master;
    vnode_t* slave;

}pty_t;

#endif