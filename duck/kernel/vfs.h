/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef VFS_H
#define VFS_H

#include "arch/arch.h"
#include "kernel/memory.h"
#include "kernel/stdarg.h"
#include "types.h"
#include "config.h"

#define V_FILE 0x01
#define V_DIRECTORY 0x02
#define V_CHARDEVICE 0x04
#define V_BLOCKDEVICE 0x08
#define V_PIPE 0x0c
#define V_SYMLINK 0x14
#define V_MOUNTPOINT 0x18

struct vnode;
typedef struct vnode vnode_t;

typedef struct vdirent {
  u64 ino;  // Inode number. Required by POSIX.
  u64 offset;
  u16 length;
  u8 type;
  char name[256];  // Filename.
} vdirent_t;

typedef u32 (*vread_t)(struct vnode *, u32, u32, u8 *);
typedef u32 (*vwrite_t)(struct vnode *, u32, u32, u8 *);
typedef u32 (*vopen_t)(struct vnode *,u32 mode);
typedef u32 (*vclose_t)(struct vnode *);
typedef size_t (*vioctl_t)(struct vnode *, u32 cmd, void *args);

typedef u32 (*vreaddir_t)(struct vnode *, struct vdirent *, u32);
typedef struct vnode *(*vfinddir_t)(struct vnode *, char *name);
typedef struct vnode *(*vfind_t)(struct vnode *, char *name);
typedef void (*vmount_t)(struct vnode *root, u8 *path, vnode_t *node);

typedef struct voperator {
  vread_t read;
  vwrite_t write;
  vopen_t open;
  vclose_t close;
  vreaddir_t readdir;
  vfinddir_t finddir;
  vioctl_t ioctl;
  vfind_t find;
  vmount_t mount;
} voperator_t;

typedef struct vnode {
  char *name;  // The filename.
  u32 mask;    // The permissions mask.
  u32 uid;     // The owning user.
  u32 gid;     // The owning group.
  u32 flags;   // Includes the node type. See #defines above.
  u32 inode;   // This is device-specific - provides a way for a filesystem to
               // identify files.
  u32 length;  // Size of the file, in bytes.
  u32 impl;    // An implementation-defined number.
  vnode_t *parent;  // Used by mountpoints and symlinks.
  vnode_t **child;
  vnode_t *super;
  size_t child_number;
  size_t child_size;
  void *data;
  void *device;
  voperator_t *op;
} vnode_t;

typedef struct vfs {
  int users;
  vnode_t *root;
  vnode_t *pwd;
} vfs_t;

u32 vread(vnode_t *node, u32 offset, u32 size, u8 *buffer);
u32 vwrite(vnode_t *node, u32 offset, u32 size, u8 *buffer);
u32 vopen(vnode_t *node,u32 mode);
u32 vclose(vnode_t *node);
u32 vreaddir(vnode_t *node, vdirent_t *dirent, u32 index);
vnode_t *vfinddir(vnode_t *node, char *name);
vnode_t *vfind(vnode_t *node, char *name);
vnode_t *vcreate(u8 *name, u32 flags);
void vmount(vnode_t *node, u8 *path, vnode_t *node1);
size_t vioctl(vnode_t *node, u32 cmd, void *args);

vnode_t *vfs_find(vnode_t *root, u8 *path);
void vfs_mount(vnode_t *root, u8 *path, vnode_t *node);
void vfs_add_child(vnode_t *parent, vnode_t *child);
void vfs_exten_child(vnode_t *node);
int vfs_init();
int vfs_close(vnode_t *node);
u32 vfs_open(vnode_t *node,u32 mode);
vnode_t *vfs_open_attr(vnode_t *root, u8 *name, u32 attr);
u32 vfs_readdir(vnode_t *node, vdirent_t *dirent, u32 count);
vnode_t *vfs_create_node(u8 *name, u32 flags);
u32 vfs_write(vnode_t *node, u32 offset, u32 size, u8 *buffer);
u32 vfs_read(vnode_t *node, u32 offset, u32 size, u8 *buffer);

#endif