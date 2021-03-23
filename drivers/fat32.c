/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "fat32.h"

fat32_info_t *fat32_info = NULL;

static u32 device_read(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {
  u32 ret = 0;
  device_t *dev = (device_t *)node->device;
  if (dev == NULL) {
    return ret;
  }
  dev->ioctl(dev, IOC_WRITE_OFFSET, offset);
  ret = dev->read(dev, buffer, nbytes);
  return ret;
}

static u32 device_write(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {
  u32 ret = 0;
  device_t *dev = (device_t *)node->device;
  if (dev == NULL) {
    return ret;
  }
  dev->ioctl(dev, IOC_WRITE_OFFSET, offset);
  ret = dev->write(dev, buffer, nbytes);
  return ret;
}

size_t fat32_read_bytes(vnode_t *node, u32 offset, size_t nbytes, u8 *buf) {
  u32 count = nbytes / BYTE_PER_SECTOR;
  u32 rest = nbytes % BYTE_PER_SECTOR;
  char small_buf[BYTE_PER_SECTOR];
  u32 ret = 0;
  for (int i = 0; i < count; i++) {
    ret = device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    kmemmove(buf, small_buf + offset % BYTE_PER_SECTOR, BYTE_PER_SECTOR);
    buf += BYTE_PER_SECTOR;
    offset += BYTE_PER_SECTOR;
  }
  if (rest > 0) {
    ret = device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    buf += count * BYTE_PER_SECTOR;
    kmemmove(buf, small_buf, rest);
  }
  if (ret < 0) {
    return -1;
  }
  return nbytes;
}

size_t fat32_write_bytes(vnode_t *node, u32 offset, size_t nbytes, u8 *buf) {
  u32 count = nbytes / BYTE_PER_SECTOR;
  u32 rest = nbytes % BYTE_PER_SECTOR;
  char small_buf[BYTE_PER_SECTOR * 2];
  u32 ret = 0;
  for (int i = 0; i < count; i++) {
    // device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    kmemmove(small_buf + offset % BYTE_PER_SECTOR, buf, BYTE_PER_SECTOR);
    ret = device_write(node, offset, BYTE_PER_SECTOR, small_buf);
    buf += BYTE_PER_SECTOR;
    offset += BYTE_PER_SECTOR;
  }
  if (rest > 0) {
    device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    kmemmove(small_buf + offset % BYTE_PER_SECTOR, buf, rest);
    ret = device_write(node, offset, BYTE_PER_SECTOR, small_buf);
    buf += count * BYTE_PER_SECTOR;
  }
  if (ret < 0) {
    return -1;
  }
  return nbytes;
};

dir_entry_t *fat32_find_file_entry(fat32_info_t *fat32, char *name,
                                   u32 *index) {
  if (fat32 == NULL) {
    kprintf("fat32_find_file_entry fat32 is null\n");
    return NULL;
  }
  char pre_name[10];
  memset(pre_name, 32, 10);
  u32 len = kstrlen(name);
  char *s = name + len - 3;
  kstrncpy(pre_name, name, len - 4);
  for (int i = 0; i < fat32->entries_number; i++) {
    dir_entry_t *e = &fat32->entries[i];
    // kprintf("%s==%s\n", pre_name, e->name);
    if (kstrncasecmp(pre_name, e->name, 8) == 0 &&
        kstrncasecmp(s, e->ext, 3) == 0) {
      *index = i;
      return e;
    }
  }
  return NULL;
}

u32 fat32_read_entries(vnode_t *node, u32 cluster, u32 *cluster_n_offset,
                       char *entry_buf) {
  if (cluster >= (*cluster_n_offset)) {
    u32 read_offset = fat32_info->fat1 + sizeof(fat32_t) * (cluster);
    memset(entry_buf, 0, 512);
    fat32_read_bytes(node, read_offset, 512, entry_buf);
    *cluster_n_offset = cluster;
    return read_offset;
  }
  return -1;
}

fat32_t *fat32_next_fat(vnode_t *node, u32 cluster, u32 *cluster_n_offset) {
  static fat32_t *fat_entry = NULL;
  if (fat_entry == NULL) {
    fat_entry = kmalloc(512);
  }
  // read entry
  fat32_read_entries(node, cluster, cluster_n_offset, fat_entry);
  u32 pos = (cluster - *cluster_n_offset);
  fat32_t *fat = &fat_entry[pos];
  return fat;
}

fat32_t fat32_find_free_fat(vnode_t *node, u32 cluster, u32 *cluster_n_offset) {
  static fat32_t *fat_entry = NULL;
  if (fat_entry == NULL) {
    fat_entry = kmalloc(512);
  }
  fat32_t *fat = NULL;
  for (int i = 0;; i++) {
    // read entry
    if (i == 0 || i > 512 / sizeof(fat32_t)) {
      fat32_read_entries(node, cluster, cluster_n_offset, fat_entry);
    }
    u32 pos = (cluster - *cluster_n_offset);
    if (*fat == FAT32_CLUSTER_FREE) {
      return cluster;
    }
    fat = &fat_entry[pos];
    cluster++;
    *cluster_n_offset = cluster;
  }
  return -1;
}

fat32_t fat32_set_fat(vnode_t *node, u32 cluster, u32 *cluster_n_offset,
                      fat32_t fat_item) {
  static fat32_t *fat_entry = NULL;
  if (fat_entry == NULL) {
    fat_entry = kmalloc(512);
  }
  // read entry
  u32 read_offset =
      fat32_read_entries(node, cluster, cluster_n_offset, fat_entry);
  u32 pos = (cluster - *cluster_n_offset);
  fat32_t *fat = &fat_entry[pos];
  fat32_t ret_fat = *fat;
  *fat = fat_item;
  fat32_write_bytes(node, read_offset, 512, fat_entry);
  return ret_fat;
}

u32 fat32_read(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {
  char *name = node->name;
  file_info_t *file_info = node->data;
  dir_entry_t *e = file_info->entry;
  if (e == NULL) {
    kprintf("read file %s entry error\n", name);
    return 0;
  }
  u32 bytes = fat32_info->bytes_per_cluster;
  static u8 *read_buffer = NULL;
  if (read_buffer == NULL) {
    read_buffer = kmalloc(bytes);
  }
#ifdef DEUBG
  kprintf("read %s ", name);
#endif
  fat32_t first_cluster =
      ((e->start_file_cluster_hight << 16) | e->start_file_cluster_low);
  fat32_t *fat = &first_cluster;
  u32 total_bytes = 0;
  u32 cluster_n_offset = 0;
  u32 n_cluster = 0;

  u32 read_offset = fat32_info->data +
                    ((*fat) - 2) * fat32_info->vbr->sector_per_cluster *
                        fat32_info->vbr->byte_per_sector +
                    offset;
  while (!FAT32_CLUSTER_IS_EOF(*fat) && *fat != 0 && total_bytes < nbytes) {
    // if (((fat32_info->bytes_per_cluster+ offset) /
    // fat32_info->bytes_per_cluster-1) <= n_cluster) {
    if ((offset / fat32_info->bytes_per_cluster) <= n_cluster) {
      memset(read_buffer, 0, bytes);
      u32 ret = fat32_read_bytes(node, read_offset, bytes, read_buffer);
      if ((nbytes - total_bytes) >= bytes) {
        kmemmove(buffer + total_bytes, read_buffer, bytes);
        total_bytes += bytes;
        read_offset += bytes;
      } else {
        u32 rest = nbytes - total_bytes;
        kmemmove(buffer + total_bytes, read_buffer, rest);
        total_bytes += rest;
        read_offset += rest;
      }
    }
#ifdef DEUBG
    kprintf("%d->", (*fat));
#endif
    fat = fat32_next_fat(node, *fat, &cluster_n_offset);
    n_cluster++;
  }
#ifdef DEUBG
  kprintf("=end total:%d\n", total_bytes);
#endif
  return total_bytes;
}

void fat32_test(vnode_t *node) {
  file_info_t *file_info = node->data;
  dir_entry_t *e = file_info->entry;
  u32 offset = 0;
  fat32_t first_cluster =
      ((e->start_file_cluster_hight << 16) | e->start_file_cluster_low);
  fat32_t *fat = &first_cluster;
  u32 read_offset = fat32_info->data +
                    ((*fat) - 2) * fat32_info->vbr->sector_per_cluster *
                        fat32_info->vbr->byte_per_sector +
                    offset;
  char *buf = kmalloc(512);
  fat32_read_bytes(node, read_offset, 512, buf);
  buf[0] = 0x11;
  buf[1] = 0x22;
  buf[2] = 0x33;
  fat32_write_bytes(node, read_offset, 512, buf);
  memset(buf, 0, 512);
  fat32_read_bytes(node, read_offset, 512, buf);
  if (buf[0] == 0x11 && buf[1] == 0x22 && buf[2] == 0x33) {
    kprintf("read write success\n");
  } else {
    kprintf("read or write error\n");
  }
}

void fat32_write_file_attr(vnode_t *node) {
  file_info_t *file_info = node->data;
  char buf[512];
  u32 offset = fat32_info->data + sizeof(dir_entry_t) * file_info->entry_index;
  fat32_read_bytes(node, offset, 512, buf);
  dir_entry_t *p = buf;
  *p = *file_info->entry;
  fat32_write_bytes(node, offset, 512, buf);
}

u32 fat32_write(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {
  char *name = node->name;
  file_info_t *file_info = node->data;
  dir_entry_t *e = file_info->entry;
  if (e == NULL) {
    kprintf("write file %s entry error\n", name);
    return 0;
  }
  u32 bytes = fat32_info->bytes_per_cluster;
  static u8 *read_buffer = NULL;
  if (read_buffer == NULL) {
    read_buffer = kmalloc(bytes);
  }

  fat32_t first_cluster =
      ((e->start_file_cluster_hight << 16) | e->start_file_cluster_low);
  fat32_t *fat = &first_cluster;
  u32 total_bytes = 0;
  u32 cluster_n_offset = 0;
  u32 n_cluster = 0;
  u32 new_fat;
  u32 read_offset = fat32_info->data +
                    ((*fat) - 2) * fat32_info->vbr->sector_per_cluster *
                        fat32_info->vbr->byte_per_sector +
                    offset;

  while (total_bytes < nbytes) {
    if ((offset / fat32_info->bytes_per_cluster) <= n_cluster) {
      memset(read_buffer, 0, bytes);
      u32 ret = fat32_read_bytes(node, read_offset, bytes, read_buffer);
      if ((nbytes - total_bytes) >= bytes) {
        kmemmove(read_buffer + total_bytes, buffer + total_bytes, bytes);
        ret = fat32_write_bytes(node, read_offset, bytes, read_buffer);
        total_bytes += bytes;
        read_offset += bytes;
      } else {
        u32 rest = nbytes - total_bytes;
        kmemmove(read_buffer + total_bytes, buffer + total_bytes, rest);
        ret = fat32_write_bytes(node, read_offset, bytes, read_buffer);
        total_bytes += rest;
        read_offset += rest;
      }
    }
    kprintf("%d->", (*fat));

    if (FAT32_CLUSTER_IS_EOF(*fat)) {
      // find new cluster
      new_fat = fat32_find_free_fat(node, *fat, &cluster_n_offset);
      kprintf("find new fat %d\n", new_fat);
      fat32_set_fat(node, *fat, &cluster_n_offset, new_fat);
      fat = &new_fat;
      n_cluster++;
      continue;
    }
    fat = fat32_next_fat(node, *fat, &cluster_n_offset);
    n_cluster++;
  }
  kprintf("\n");
  fat32_t ret_fat;
  u32 end = FAT32_CLUSTER_EOF;
  // mark free
  while (!FAT32_CLUSTER_IS_EOF(*fat) && *fat != 0) {
    kprintf("%d=>", (*fat));
    ret_fat = fat32_set_fat(node, *fat, &cluster_n_offset, end);
    fat = &ret_fat;
    end = FAT32_CLUSTER_FREE;
  }
  // mark last
  if (FAT32_CLUSTER_IS_EOF(*fat)) {
    kprintf("%d=>", (*fat));
    fat32_set_fat(node, *fat, &cluster_n_offset, FAT32_CLUSTER_FREE);
  }
  kprintf("\n");

  // todo
  // e->last_modify_date=;
  if (offset > e->file_size) {
    e->file_size = e->file_size - offset + nbytes;
  }
  fat32_write_file_attr(node);

  return total_bytes;
}

u32 fat32_open(vnode_t *node) {
  char *name = node->name;
  fat32_info_t *fat32 = node->data;
  u32 index = 0;
  dir_entry_t *e = fat32_find_file_entry(fat32, name, &index);
  if (e == NULL) {
    kprintf("open file %s error\n", name);
    return 0;
  }
  return 1;
}

vnode_t *fat32_find(vnode_t *node, char *name) {
  fat32_info_t *fat32 = node->data;
  u32 index = 0;
  dir_entry_t *e = fat32_find_file_entry(fat32, name, &index);
  if (e == NULL) {
    kprintf("find file %s error\n", name);
    return NULL;
  }
  // kprintf("find cluster:%d\n",e->start_file_cluster_low);
  vnode_t *file = vfs_create(name, V_FILE);
  file_info_t *file_info = kmalloc(sizeof(file_info_t));
  file_info->entry = e;
  file_info->entry_index = index;
  file->data = file_info;
  file->device = node->device;
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
  fat32_read_bytes(node, 0, sizeof(vbr_t), fat32_info->vbr);

  fat32_info->fs_info = kmalloc(sizeof(fs_info_t));
  fat32_read_bytes(node, fat32_info->vbr->fs_info * 512, sizeof(fs_info_t),
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

  u32 fat_size =
      fat32_info->vbr->sector_per_fat_number * fat32_info->vbr->byte_per_sector;
  fat32_info->fat_size = fat_size;
  fat32_info->fat1 = fat1;
  fat32_info->fat2 = fat2;
  fat32_info->root_dir = root_dir;
  fat32_info->data = data;

  fat32_info->entries_number = 80;
  dir_entry_t *entries =
      kmalloc(sizeof(dir_entry_t) * fat32_info->entries_number);
  fat32_read_bytes(node, data, sizeof(dir_entry_t) * fat32_info->entries_number,
                   entries);
  fat32_info->entries = entries;

  fat32_info->bytes_per_cluster =
      fat32_info->vbr->sector_per_cluster * fat32_info->vbr->byte_per_sector;

  return 0;
}

void fat32_exit(void) { kprintf("fat32 exit\n"); }

module_t fat32_module = {
    .name = "fat32", .init = fat32_init, .exit = fat32_exit};