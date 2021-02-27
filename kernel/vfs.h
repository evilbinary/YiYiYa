/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef VFS_H
#define VFS_H

#include "arch/arch.h"
#include "kernel/stdarg.h"

#define V_FILE        0x01
#define V_DIRECTORY   0x02
#define V_CHARDEVICE  0x03
#define V_BLOCKDEVICE 0x04
#define V_PIPE        0x05
#define V_SYMLINK     0x06
#define V_MOUNTPOINT  0x08

struct vnode;
typedef struct vnode vnode_t;

struct vdirent{
  char name[128]; // Filename.
  u32 ino;     // Inode number. Required by POSIX.
};

typedef u32 (*vread_t)(struct vnode*,u32,u32,u8*);
typedef u32 (*vwrite_t)(struct vnode*,u32,u32,u8*);
typedef u32 (*vopen_t)(struct vnode*);
typedef void (*vclose_t)(struct vnode*);
typedef size_t (*vioctl_t)(struct vnode*,u32 cmd, ...);

typedef struct vdirent * (*vreaddir_t)(struct vnode*,u32);
typedef struct vnode * (*vfinddir_t)(struct vnode*,char *name);
typedef struct vnode * (*vfind_t)(struct vnode*,char *name);
typedef void (*vmount_t)(struct vnode* root,u8* path,vnode_t* node);

typedef struct vnode{
   char *name;  // The filename.
   u32 mask;        // The permissions mask.
   u32 uid;         // The owning user.
   u32 gid;         // The owning group.
   u32 flags;       // Includes the node type. See #defines above.
   u32 inode;       // This is device-specific - provides a way for a filesystem to identify files.
   u32 length;      // Size of the file, in bytes.
   u32 impl;        // An implementation-defined number.
   vread_t read;
   vwrite_t write;
   vopen_t open;
   vclose_t close;
   vreaddir_t readdir;
   vfinddir_t finddir;
   vioctl_t ioctl;
   vfind_t find;
   vmount_t mount;
   vnode_t *parent; // Used by mountpoints and symlinks.
   vnode_t **child;
   size_t child_number;
   size_t child_size;
   void* data;
   void* device;
} vnode_t;


u32 vread(vnode_t *node, u32 offset, u32 size, u8 *buffer);
u32 vwrite(vnode_t *node, u32 offset, u32 size, u8 *buffer);
u32 vopen(vnode_t *node);
void vclose(vnode_t *node);
struct vdirent *vreaddir(vnode_t *node, u32 index);
vnode_t *vfinddir(vnode_t *node, char *name);
vnode_t *vfind(vnode_t *node, char *name);
vnode_t *vcreate(u8 *name, u32 flags);
void vmount(vnode_t* node,u8 *path, vnode_t *node1);
size_t vioctl(vnode_t *node, u32 cmd, ...);

vnode_t* vfs_find(vnode_t* root,u8 *path);
void vfs_mount(vnode_t *root, u8 *path, vnode_t *node);
vnode_t *vfs_create(u8 *name, u32 flags);
void vfs_add_child(vnode_t *parent, vnode_t *child);
void vfs_exten_child(vnode_t *node);


#endif