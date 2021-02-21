/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "fat32.h"

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
  for (int i = 0; i < fat32->entries_number; i++) {
    dir_entry_t *e = &fat32->entries[i];
    if (kstrcmp(name, e->name) == 0) {
      return e;
    }
  }
  return NULL;
}

u32 fat32_read(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {
  char *name = node->name;
  fat32_info_t *fat32 = node->data;
  dir_entry_t *e = fat32_find_file_entry(fat32, name);
  if (e == NULL) {
    kprintf("read file %s error\n", name);
    return 0;
  }
  fat32_t *fat_entry = fat32->fat1;
  fat_entry += e->start_file_cluster;

  while (*fat_entry != FAT_EOC) {
    u32 offset = (*fat_entry) * fat32->vbr->sector_per_cluster *
                 fat32->vbr->byte_per_sector;
    read(node, offset,
         fat32->vbr->sector_per_cluster * fat32->vbr->byte_per_sector, buffer);
    buffer += fat32->vbr->sector_per_cluster * fat32->vbr->byte_per_sector;
    fat_entry = *fat_entry;
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
  return 0;
}

void fat32_close(vnode_t *node) {
  
}

int fat32_init(void) {
  vnode_t *node = vfs_find(NULL, "/dev/sda");
  if (node == NULL) {
    kprintf("not found sda\n");
  }
  node->read = fat32_read;
  node->write = fat32_write;
  node->open = fat32_open;
  node->close = fat32_close;

  fat32_info_t *fat32 = kmalloc(sizeof(fat32_info_t));
  node->data = fat32;

  fat32->vbr = kmalloc(sizeof(vbr_t));
  fat32_read(node, 0, sizeof(vbr_t), fat32->vbr);

  fat32->fs_info = kmalloc(sizeof(fs_info_t));
  fat32_read(node, fat32->vbr->fs_info * 512, sizeof(fs_info_t),
             fat32->fs_info);

  u32 reserved =
      fat32->vbr->reserved_sector_count * fat32->vbr->byte_per_sector;
  u32 fat1 = reserved;
  u32 fat2 = reserved +
             fat32->vbr->sector_per_fat_number * fat32->vbr->byte_per_sector;
  u32 root_dir = (fat32->vbr->reserved_sector_count +
                  fat32->vbr->sector_per_fat_number * fat32->vbr->fat_number) *
                 fat32->vbr->byte_per_sector;
  u32 data =
      root_dir + fat32->vbr->root_dir_number * 32;  // only for fat16 fat32

  fat32->fat1 = fat1;
  fat32->fat2 = fat2;
  fat32->root_dir = root_dir;
  fat32->data = data;

  fat32->entries_number = 20;
  dir_entry_t *entries = kmalloc(sizeof(dir_entry_t) * fat32->entries_number);
  fat32_read(node, data, sizeof(dir_entry_t) * fat32->entries_number, entries);
  fat32->entries = entries;

  return 0;
}

void fat32_exit(void) { kprintf("fat32 exit\n"); }

module_t fat32_module = {
    .name = "fat32", .init = fat32_init, .exit = fat32_exit};