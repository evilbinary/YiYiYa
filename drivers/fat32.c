/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "fat32.h"

fat32_info_t *fat32_info = NULL;

static u32 read(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {
  u32 ret = 0;
  device_t *dev = (device_t *)node->device;
  if (dev == NULL) {
    return ret;
  }
  dev->ioctl(dev, IOC_WRITE_OFFSET, offset);
  ret = dev->read(dev, buffer, nbytes);
  return ret;
}

static u32 write(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {
  u32 ret = 0;
  device_t *dev = (device_t *)node->device;
  if (dev == NULL) {
    return ret;
  }
  dev->ioctl(dev, IOC_WRITE_OFFSET, offset);
  ret = dev->write(dev, buffer, nbytes);
  return ret;
}

dir_entry_t *fat32_find_file_entry(fat32_info_t *fat32, char *name) {
  if (fat32 == NULL) {
    kprintf("fat32_find_file_entry fat32 is null\n");
    return NULL;
  }
  char pre_name[8];
  memset(pre_name, 32, 8);
  u32 len = kstrlen(name);
  char *s = name + len - 3;
  kstrncpy(pre_name, name, len - 4);
  for (int i = 0; i < fat32->entries_number; i++) {
    dir_entry_t *e = &fat32->entries[i];
    if (kstrncmp(pre_name, e->name, 8) == 0 && kstrncmp(s, e->ext, 3) == 0) {
      return e;
    }
  }
  return NULL;
}

u32 fat32_read(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {
  char *name = node->name;
  dir_entry_t *e = node->data;
  if (e == NULL) {
    kprintf("read file %s entry error\n", name);
    return 0;
  }

  char buf[512];
  //read entry
  read(node, fat32_info->fat1, 512, buf);
  u8* read_buffer=buffer;
  fat32_t first_number=e->start_file_cluster_low-2;
  fat32_t *fat_entry = buf;
  fat32_t* fat_n=&first_number;
  u32 total_bytes=0;
  while (*fat_n != FAT_EOC&&total_bytes<nbytes) {
    u32 offset =fat32_info->data+ (*fat_n) * fat32_info->vbr->sector_per_cluster *
                 fat32_info->vbr->byte_per_sector;
    u32 bytes =
        fat32_info->vbr->sector_per_cluster * fat32_info->vbr->byte_per_sector;
    u32 ret=read(node, offset, bytes, read_buffer);
    total_bytes+=ret;
    read_buffer += fat32_info->vbr->sector_per_cluster * fat32_info->vbr->byte_per_sector;
    fat_n = &fat_entry[*fat_n];
  }
}

u32 fat32_write(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {}

u32 fat32_open(vnode_t *node) {
  char *name = node->name;
  fat32_info_t *fat32 = node->data;
  dir_entry_t *e = fat32_find_file_entry(fat32, name);
  if (e == NULL) {
    kprintf("open file %s error\n", name);
    return 0;
  }
  return 1;
}

vnode_t *fat32_find(vnode_t *node, char *name) {
  fat32_info_t *fat32 = node->data;
  dir_entry_t *e = fat32_find_file_entry(fat32, name);
  if (e == NULL) {
    kprintf("find file %s error\n", name);
    return NULL;
  }

  vnode_t *file = vfs_create(name, V_FILE);
  file->data = e;
  file->device=node->device;
  fat32_init_op(file);
  return file;
}

void fat32_init_op(vnode_t *node) {
  node->read = fat32_read;
  node->write = fat32_write;
  node->open = fat32_open;
  node->close = fat32_close;
  node->find = fat32_find;
}

void fat32_close(vnode_t *node) {}

int fat32_init(void) {
  vnode_t *node = vfs_find(NULL, "/dev/sda");
  if (node == NULL) {
    kprintf("not found sda\n");
  }
  fat32_init_op(node);

  fat32_info = kmalloc(sizeof(fat32_info_t));
  node->data = fat32_info;

  fat32_info->vbr = kmalloc(sizeof(vbr_t));
  read(node, 0, sizeof(vbr_t), fat32_info->vbr);

  fat32_info->fs_info = kmalloc(sizeof(fs_info_t));
  read(node, fat32_info->vbr->fs_info * 512, sizeof(fs_info_t),
       fat32_info->fs_info);

  u32 reserved =
      fat32_info->vbr->reserved_sector_count * fat32_info->vbr->byte_per_sector;
  u32 fat1 = reserved;
  u32 fat2 = reserved + fat32_info->vbr->sector_per_fat_number *
                            fat32_info->vbr->byte_per_sector;
  u32 root_dir =
      (fat32_info->vbr->reserved_sector_count +
       fat32_info->vbr->sector_per_fat_number * fat32_info->vbr->fat_number) *
      fat32_info->vbr->byte_per_sector;
  u32 data =
      root_dir + fat32_info->vbr->root_dir_number * 32;  // only for fat16 fat32

  u32 fat_size=fat32_info->vbr->sector_per_fat_number *fat32_info->vbr->byte_per_sector;
  fat32_info->fat_size=fat_size;
  fat32_info->fat1 = fat1;
  fat32_info->fat2 = fat2;
  fat32_info->root_dir = root_dir;
  fat32_info->data = data;

  fat32_info->entries_number = 20;
  dir_entry_t *entries =
      kmalloc(sizeof(dir_entry_t) * fat32_info->entries_number);
  read(node, data, sizeof(dir_entry_t) * fat32_info->entries_number, entries);
  fat32_info->entries = entries;

  return 0;
}

void fat32_exit(void) { kprintf("fat32 exit\n"); }

module_t fat32_module = {
    .name = "fat32", .init = fat32_init, .exit = fat32_exit};