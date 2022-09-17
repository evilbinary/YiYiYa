/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "vfs.h"

#include "fd.h"

vnode_t *root_node = NULL;
voperator_t default_operator = {.write = vfs_write,
                                .read = vfs_read,
                                .close = vfs_close,
                                .open = vfs_open,
                                .find = vfs_find,
                                .mount = vfs_mount,
                                .readdir = vfs_readdir};

size_t vioctl(vnode_t *node, u32 cmd, void *args) {
  if (node->op->ioctl != NULL) {
    u32 ret = 0;
    // va_list args;
    // va_start(args, cmd);
    ret = node->op->ioctl(node, cmd, args);
    // va_end(args);
    return ret;
  } else {
    kprintf("node %s ioctl is null\n", node->name);
    return 0;
  }
}

u32 vread(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  if (node->op->read != NULL) {
    return node->op->read(node, offset, size, buffer);
  } else {
    kprintf("node %s read is null\n", node->name);
    return 0;
  }
}

u32 vwrite(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  if (node->op->write != NULL) {
    return node->op->write(node, offset, size, buffer);
  } else {
    kprintf("node %s write is null\n", node->name);
    return 0;
  }
}

u32 vopen(vnode_t *node,u32 mode) {
  if (node->op->open != NULL) {
    return node->op->open(node,mode);
  } else {
    kprintf("node %s open is null \n", node->name);
    return -1;
  }
}
u32 vclose(vnode_t *node) {
  if (node->op->close != NULL) {
    return node->op->close(node);
  } else {
    kprintf("node %s close is null\n", node->name);
    return -1;
  }
}
u32 vreaddir(vnode_t *node, vdirent_t *dirent, u32 count) {
  if (node->op->readdir != NULL) {
    if ((node->flags & V_DIRECTORY) == V_DIRECTORY) {
      return node->op->readdir(node, dirent, count);
    } else {
      kprintf("node readdir is not dir\n");
    }
  } else {
    kprintf("node readdir is null\n");
    return 0;
  }
}
vnode_t *vfinddir(vnode_t *node, char *name) {
  if (node->op->finddir != NULL != NULL) {
    if ((node->flags & V_DIRECTORY) == V_DIRECTORY) {
      return node->op->finddir(node, name);
    } else {
      kprintf("node finddir is not dir\n");
    }
  } else {
    kprintf("node finddir is null\n");
    return 0;
  }
}

vnode_t *vfind(vnode_t *node, char *name) {
  if (node == NULL) {
    node = root_node;
  }
  if (node->op->find != NULL) {
    return node->op->find(node, name);
  } else {
    kprintf("node find is null\n");
    return 0;
  }
}

void vmount(vnode_t *root, u8 *path, vnode_t *node) {
  if (root->op->mount != NULL) {
    return node->op->mount(root, path, node);
  } else {
    kprintf("node mount is null\n");
    return;
  }
}

void vfs_exten_child(vnode_t *node) {
  u32 size = 4;
  if (node->child_number != 0) {
    size = node->child_number * 2;
  }
  vnode_t **child = kmalloc(size * sizeof(vnode_t *));
  for (int i = 0; i < size; i++) {
    child[i] = 0;
  }
  vnode_t **temp = node->child;
  if (node->child != NULL) {
    kmemmove(child, node->child, node->child_number * sizeof(vnode_t *));
    kfree(temp);
  }
  node->child = child;
  node->child_size = size;
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
  char buf[MAX_PATH_BUFFER];
  char *start;
  char *s = buf;

  if (root == NULL) {
    root = root_node;
  }
  u32 path_len = kstrlen(path);
  if (path_len == 1 && kstrcmp(root->name, path) == 0) {
    return root;
  }
  if (path_len >= MAX_PATH_BUFFER) {
    s = kmalloc(path_len);
    start = s;
  }
  kstrcpy(s, path);
  token = kstrtok(s, split);

  vnode_t *parent = root;
  vnode_t *node = NULL;
  if (token == NULL) {
    node = parent;
  }
  while (token != NULL) {
    // if (kstrcmp(token, parent->name) == 0) {
    //   continue;
    // }
    for (int i = 0; i < parent->child_number; i++) {
      vnode_t *n = parent->child[i];
      if (n == NULL) continue;
      if (kstrcmp(token, n->name) == 0) {
        node = n;
        parent = n;
        break;
      }
    }
    token = kstrtok(NULL, split);
  }
  if (path_len >= MAX_PATH_BUFFER) {
    kfree(start);
  }
  // not found vfs vnode,is super block then find on block
  if (node == NULL && parent->super != NULL) {
    kstrcpy(s, path);
    token = kstrtok(s, split);
    if (kstrcmp(split, parent->name) == 0) {
    } else {
      while (token != NULL) {
        if (kstrcmp(token, parent->name) == 0) {
          break;
        }
        token = kstrtok(NULL, split);
      }
    }

    vnode_t *find_node = vfind(parent->super, token);
    if (find_node == NULL) {
      kprintf("open find %s failed \n", path);
      return NULL;
    }
    node = find_node;
    // mount to vfs
    vfs_add_child(parent, node);
  }

  return node;
}

void vfs_mount(vnode_t *root, u8 *path, vnode_t *node) {
  if (root == NULL) {
    root = root_node;
  }
  vnode_t *parent = vfs_find(root, path);
  if (parent != NULL) {
    vfs_add_child(parent, node);
  } else {
    kprintf("mount on %s error\n", path);
  }
}

u32 vfs_readdir(vnode_t *node, vdirent_t *dirent, u32 count) {
  // todo search int vfs
  if (node->super != NULL) {
    return node->super->op->readdir(node->super, dirent, count);
  }
  return 0;
}

u32 vfs_write(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  return vwrite(node, offset, size, buffer);
}

u32 vfs_read(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  return vread(node, offset, size, buffer);
}

u32 vfs_open(vnode_t *node,u32 mode) {
  int ret = 0;
  if (node->super != NULL) {
    node->super->op->open(node,mode);
  }
  return ret;
}

vnode_t *vfs_create_node(u8 *name, u32 flags) {
  vnode_t *node = kmalloc(sizeof(vnode_t));
  node->name = kmalloc(kstrlen(name) + 1);
  kstrcpy(node->name, name);
  node->flags = flags;
  node->op = &default_operator;

  node->child = NULL;
  node->child_number = 0;
  node->child_size = 0;
  return node;
}

vnode_t *vfs_open_attr(vnode_t *root, u8 *name, u32 attr) {
  if (name == NULL) {
    return root;
  }
  if (root == NULL) {
    root = root_node;
  }
  vnode_t *file = NULL;
  if ((attr & O_CREAT) == O_CREAT) {
    char parent_path[MAX_PATH_BUFFER];
    int len = kstrlen(name);
    while (len > 0) {
      if (name[--len] == '/') {
        break;
      }
    }
    kstrncpy(parent_path, name, len + 1);
    parent_path[len + 1] = 0;
    vnode_t *node = vfs_find(root, parent_path);
    if (node == NULL) {
      kprintf("open parent %s %s failed \n", parent_path, name);
      return NULL;
    }
    char *last = kstrrstr(name, root->name);
    if (last != NULL) {
      last += kstrlen(node->name);
      if (last[0] == '/') last++;
    }
    file = vfs_create_node(last, V_FILE);
    file->device = node->device;
    file->data = node->data;
    if(node->super!=NULL){
      if (node->super->op != NULL) {
        file->op = node->super->op;
      }
    }else{
      file->op = node->parent->op;
    }
    file->super = node->super;
    vfs_add_child(node, file);
  } else {
    vnode_t *node = vfs_find(root, name);
    if (node == NULL) {
      kprintf("open file %s failed \n", name);
      return NULL;
    }
    file = node;
  }
  u32 ret = vfs_open(file,attr);
  if (ret < 0) {
    kprintf("open third %s failed \n", name);
    return NULL;
  }
  return file;
}

int vfs_close(vnode_t *node) {
  if (node == NULL) {
    kprintf("close node is nul\n");
    return -1;
  }
  if (node->super != NULL) {
    return vclose(node->super);
  }
  return 0;
}

int vfs_init() {
  root_node = vfs_create_node("/", V_DIRECTORY);
  return 1;
}