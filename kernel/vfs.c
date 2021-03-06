/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "vfs.h"

#include "fd.h"

vnode_t *root_node = NULL;

size_t vioctl(vnode_t *node, u32 cmd, va_list args) {
  if (node->ioctl != NULL) {
    u32 ret = 0;
    // va_list args;
    // va_start(args, cmd);
    ret = node->ioctl(node, cmd, args);
    // va_end(args);
    return ret;
  } else {
    return 0;
  }
}

u32 vread(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  if (node->read != NULL) {
    return node->read(node, offset, size, buffer);
  } else {
    return 0;
  }
}
u32 vwrite(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  if (node->write != NULL) {
    return node->write(node, offset, size, buffer);
  } else {
    return 0;
  }
}
u32 vopen(vnode_t *node) {
  if (node->open != NULL) {
    return node->open(node);
  } else {
    return;
  }
}
void vclose(vnode_t *node) {
  if (node->close != NULL) {
    return node->close(node);
  } else {
    return;
  }
}
struct vdirent *vreaddir(vnode_t *node, u32 index) {
  if ((node->flags & 0x7) == V_DIRECTORY && node->readdir != NULL) {
    return node->readdir(node, index);
  } else {
    return 0;
  }
}
vnode_t *vfinddir(vnode_t *node, char *name) {
  if ((node->flags & 0x7) == V_DIRECTORY && node->finddir != NULL) {
    return node->finddir(node, name);
  } else {
    return 0;
  }
}

vnode_t *vfind(vnode_t *node, char *name) {
  if (node == NULL) {
    node = root_node;
  }
  if (node->find != NULL) {
    return node->find(node, name);
  } else {
    return 0;
  }
}

void vmount(vnode_t *root, u8 *path, vnode_t *node) {
  if (root->mount != NULL) {
    return node->mount(root, path, node);
  } else {
    return;
  }
}

void vfs_exten_child(vnode_t *node) {
  u32 size = 4;
  if (node->child_number != 0) {
    size = node->child_number * 2;
  }
  vnode_t **child = kmalloc(size);
  kmemmove(child, node->child, node->child_number * sizeof(vnode_t *));
  vnode_t **temp = node->child;
  node->child = child;
  node->child_size = size;
  kfree(temp);
}

void vfs_add_child(vnode_t *parent, vnode_t *child) {
  if ((parent->child_number + 1) > parent->child_size) {
    vfs_exten_child(parent);
  }
  if (parent->child == NULL) {
    kprintf("child alloc error\n");
    return;
  }
  child->parent = parent;
  parent->child[parent->child_number++] = child;
}

vnode_t *vfs_find(vnode_t *root, u8 *path) {
  char *token;
  const char *split = "/";
  char buf[256];
  char *start;
  char *s = buf;
  u32 path_len = kstrlen(path);
  if (path_len >= 256) {
    s = kmalloc(path_len);
    start = s;
  }
  kstrcpy(s, path);
  token = kstrtok(s, split);
  if (root == NULL) {
    root = root_node;
  }
  vnode_t *parent = root;
  while (token != NULL) {
    // if (kstrcmp(token, parent->name) == 0) {
    //   continue;
    // }
    for (int i = 0; i < parent->child_number; i++) {
      vnode_t *n = parent->child[i];
      if (n == NULL) continue;
      if (kstrcmp(token, n->name) == 0) {
        parent = n;
        break;
      }
    }
    token = kstrtok(NULL, split);
  }
  if (path_len >= 256) {
    kfree(start);
  }
  return parent;
}

void vfs_mount(vnode_t *root, u8 *path, vnode_t *node) {
  if (root_node == NULL) {
    root_node = vfs_create("/", V_MOUNTPOINT);
  }
  if (root == NULL) {
    root = root_node;
  }
  vnode_t *parent = vfs_find(root, path);
  vfs_add_child(parent, node);
}

vnode_t *vfs_create(u8 *name, u32 flags) {
  vnode_t *node = kmalloc(sizeof(vnode_t));
  node->name = name;
  node->flags = flags;
  node->find = vfs_find;
  node->mount = vfs_mount;
  return node;
}

vnode_t *vfs_open(vnode_t *root, u8 *name, u32 attr) {
  if (name == NULL) {
    return root;
  }
  if (root == NULL) {
    root = root_node;
  }
  vnode_t *node = vfind(root, name);
  if (node == NULL) {
    kprintf("open %s failed \n", name);
    return NULL;
  }
  char *last = kstrrstr(name, node->name);
  if (last != NULL) {
    last += kstrlen(node->name);
    if (last[0] == '/') last++;
  }
  vnode_t *file = vfind(node, last);
  if (file == NULL) {
    if (attr & O_CREAT == O_CREAT) {
      file = vfs_create(last, V_FILE);
      file->device = node->device;
      file->data=node->data;
      file->open = node->open;
    } else {
      kprintf("open %s failed \n", name);
      return NULL;
    }
  }
  file->parent=node;
  u32 ret = vopen(file);
  if (ret < 0) {
    kprintf("open %s failed \n", name);
    return NULL;
  }
  return file;
}

void vfs_close(vnode_t *node) {
  if (node == NULL) {
    kprintf("close node is nul\n");
    return;
  }
  vclose(node);
}

int vfs_init() { return 1; }