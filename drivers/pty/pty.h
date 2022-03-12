/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef PTY_H
#define PTY_H

#include "kernel/kernel.h"
#include "algorithm/buffer.h"

#define PTY_BUFFER 256

#define IOC_PTY_MAGIC 'p'

#define IOC_SLAVE _IOW(IOC_PTY_MAGIC, 1, int)
#define IOC_READ_OFFSET _IOW(IOC_PTY_MAGIC, 3, int)
#define IOC_WRITE_OFFSET _IOW(IOC_PTY_MAGIC, 4, int)
#define IOC_MASTER_READ_NOBLOCK _IOW(IOC_PTY_MAGIC, 5, int)
typedef struct pty{
    buffer_t *in;
    buffer_t* out;

    vnode_t* master;
    vnode_t* slave;

    u32 master_read_block;
}pty_t;

u32 pty_master_write(vnode_t *node, u32 offset, u32 size, u8 *buffer);
u32 pty_master_read(vnode_t *node, u32 offset, u32 size, u8 *buffer);

u32 pty_slave_read(vnode_t *node, u32 offset, u32 size, u8 *buffer);
u32 pty_slave_open(vnode_t *node);
u32 pty_slave_write(vnode_t *node, u32 offset, u32 size, u8 *buffer);


u32 pty_open(vnode_t *node);
u32 pty_close(vnode_t *node);
size_t pty_ioctl(vnode_t *node, u32 cmd, va_list args);



#endif