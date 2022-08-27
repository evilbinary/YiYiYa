/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "fat32.h"

fat32_info_t *fat32_info = NULL;

static u32 fat32_device_read(vnode_t *node, u32 offset, size_t nbytes,
                             u8 *buffer) {
  u32 ret = 0;
  device_t *dev = (device_t *)node->device;
  if (dev == NULL) {
    return ret;
  }
  dev->ioctl(dev, IOC_WRITE_OFFSET, offset);
  ret = dev->read(dev, buffer, nbytes);
  return ret;
}

static u32 fat32_device_write(vnode_t *node, u32 offset, size_t nbytes,
                              u8 *buffer) {
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
    ret = fat32_device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    kmemmove(buf, small_buf + offset % BYTE_PER_SECTOR, BYTE_PER_SECTOR);
    buf += BYTE_PER_SECTOR;
    offset += BYTE_PER_SECTOR;
  }
  if (rest > 0) {
    ret = fat32_device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    buf += count * BYTE_PER_SECTOR;
    kmemmove(buf, small_buf + offset % BYTE_PER_SECTOR, rest);
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
  kmemset(small_buf, 0, BYTE_PER_SECTOR);
  u32 ret = 0;
  for (int i = 0; i < count; i++) {
    fat32_device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    kmemmove(small_buf + offset % BYTE_PER_SECTOR, buf, BYTE_PER_SECTOR);
    ret = fat32_device_write(node, offset, BYTE_PER_SECTOR, small_buf);
    buf += BYTE_PER_SECTOR;
    offset += BYTE_PER_SECTOR;
  }
  if (rest > 0) {
    fat32_device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    kmemmove(small_buf + offset % BYTE_PER_SECTOR, buf, rest);
    ret = fat32_device_write(node, offset, BYTE_PER_SECTOR, small_buf);
    buf += count * BYTE_PER_SECTOR;
  }
  if (ret < 0) {
    return -1;
  }
  return nbytes;
};

int fat32_get_long_file_name(lfn_entry_t *lfn, char *lnf_name) {
  u32 max_fn = (lfn->seq) - 0x40;
  lfn = lfn + max_fn - 1;
  int j = 0, k;
  kmemset(lnf_name, 0, 256);
  for (int i = max_fn; i > 0; i--) {
    for (k = 0; k < 5; k++) {
      lnf_name[j++] = ((u16 *)lfn->name1)[k];
    }
    for (k = 0; k < 6; k++) {
      lnf_name[j++] = ((u16 *)lfn->name2)[k];
    }
    for (k = 0; k < 2; k++) {
      lnf_name[j++] = ((u16 *)lfn->name3)[k];
    }
    lfn--;
  }
  lnf_name[j++] = '\x00';
  return j;
}

u32 cmp_short_name(char *name, dir_entry_t *e) {
  u32 ret = 0;
  u32 len = kstrlen(name);
  char pre_name[10];
  kmemset(pre_name, 32, 8);
  int ext = 0;
  char *pos = kstrchr(name, '.');
  if (pos != NULL) {
    ext = 3;
    kstrncpy(pre_name, name, len - ext - 1);
    char *s = name + len - ext;
    ret = kstrncasecmp(pre_name, e->name, 8) == 0 &&
          kstrncasecmp(s, e->ext, 3) == 0;
  } else {
    kstrncpy(pre_name, name, len);
    ret = kstrncasecmp(pre_name, e->name, 8) == 0 && e->ext[0] == 32 &&
          e->ext[1] == 32;
  }
  // kprintf("%s==%s\n", pre_name, e->name);
  return ret;
}

dir_entry_t *fat32_find_file_entry(dir_entry_t *entries, u32 entries_number,
                                   char *name, u32 *index) {
  if (entries == NULL) {
    kprintf("entries  is null\n");
    return NULL;
  }
  char lnf_name[256];
  for (int i = 0; i < entries_number; i++) {
    dir_entry_t *e = &entries[i];
    lfn_entry_t *lfn = e;
    if (e->attr != FAT32_ATTR_LONG_FILE_NAME) {
      u32 ret = cmp_short_name(name, e);
      if (ret) {
        *index = i;
        return e;
      }
    } else if ((lfn->seq & 0xc0) == 0x40) {
      int j = fat32_get_long_file_name(lfn, lnf_name);
      // kprintf("lfn %s==%s\n", name, lnf_name);
      if (kstrncasecmp(name, lnf_name, j) == 0) {
        *index = (lfn->seq) - 0x40 + i;
        return lfn + (lfn->seq) - 0x40;
      }
    }
  }
  return NULL;
}

fat32_t *fat32_next_fat(vnode_t *node, u32 cluster, u32 *cluster_n_offset) {
  static fat32_t *fat_entry = NULL;
  if (fat_entry == NULL) {
    fat_entry = kmalloc(512);
  }
  // // read entry
  // u32 read_offset = cluster_fat(cluster);
  // fat32_read_bytes(node, read_offset, 512, fat_entry);

  if (cluster >= (*cluster_n_offset)) {
    u32 read_offset = fat32_info->fat1 + sizeof(fat32_t) * (cluster);
    kmemset(fat_entry, 0, 512);
    fat32_read_bytes(node, read_offset, 512, fat_entry);
    *cluster_n_offset = cluster;
  }

  u32 pos = (cluster - *cluster_n_offset);
  fat32_t *fat = &fat_entry[pos];
  return fat;
}

u32 fat32_find_free_fat(vnode_t *node, u32 cluster) {
  static fat32_t *fat_entry = NULL;
  if (fat_entry == NULL) {
    fat_entry = kmalloc(512);
  }
  fat32_t *fat = NULL;
  u32 cluster_count = fat32_info->fat_size / sizeof(fat32_t);
  for (int i = 0; i < cluster_count; i++) {
    // read entry
    if (i == 0) {
      u32 read_offset = cluster_fat(cluster++);
      fat32_read_bytes(node, read_offset, 512, fat_entry);
    }
    fat = &fat_entry[i];
    kprintf("%d %x\n", i, *fat);
    if ((*fat & 0x0fffffff) == FAT32_CLUSTER_FREE) {
      return i;
    }
  }
  return -1;
}

void fat32_printf_buffer(char *str, char *buffer, size_t size) {
  kprintf("=====>%s\n", str);
  for (int i = 0; i < size; i++) {
    kprintf("%c", buffer[i]);
  }
  kprintf("\n======\n\n");
}

fat32_t fat32_set_fat(vnode_t *node, u32 cluster, u32 fat_index,
                      fat32_t fat_item) {
  static fat32_t *fat_entry = NULL;
  if (fat_entry == NULL) {
    fat_entry = kmalloc(512);
  }
  // read entry
  u32 read_offset = cluster_fat(cluster);
  fat32_read_bytes(node, read_offset, 512, fat_entry);
  fat32_t *fat = &fat_entry[fat_index];
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
  fat32_t first_cluster = cluster_no(e);
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
      kmemset(read_buffer, 0, bytes);
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
  kmemset(buf, 0, 512);
  fat32_read_bytes(node, read_offset, 512, buf);
  if (buf[0] == 0x11 && buf[1] == 0x22 && buf[2] == 0x33) {
    kprintf("read write success\n");
  } else {
    kprintf("read or write error\n");
  }
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

  fat32_t first_cluster = cluster_no(e);
  fat32_t *fat = &first_cluster;
  u32 total_bytes = 0;
  u32 cluster_n_offset = 0;
  u32 n_cluster = 0;
  u32 new_fat;

  u32 read_offset = fat32_info->data +
                    ((*fat) - 2) * fat32_info->vbr->sector_per_cluster *
                        fat32_info->vbr->byte_per_sector +
                    offset;
  kprintf("first cluster %d wirte offset %d\n", first_cluster, read_offset);

  while (total_bytes < nbytes) {
    if ((offset / fat32_info->bytes_per_cluster) <= n_cluster) {
      kmemset(read_buffer, 0, bytes);
      u32 ret = fat32_read_bytes(node, read_offset, bytes, read_buffer);

      fat32_printf_buffer("read offset=>", read_buffer, bytes);

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
      u32 new_fat_index = fat32_find_free_fat(node, *fat);
      kprintf("find new fat %d\n", new_fat_index);
      fat32_set_fat(node, *fat, &cluster_n_offset, new_fat_index);
      fat = &new_fat_index;
      n_cluster++;
      continue;
    }
    fat = fat32_next_fat(node, *fat, &cluster_n_offset);
    n_cluster++;
  }
  kprintf(" end \n");
  fat32_t ret_fat;
  u32 end = FAT32_CLUSTER_EOF;
  // // mark free
  // while (!FAT32_CLUSTER_IS_EOF(*fat) && *fat != 0) {
  //   kprintf("%d=>", (*fat));
  //   ret_fat = fat32_set_fat(node, *fat,cluster_n_offset , end);
  //   fat = &ret_fat;
  //   end = FAT32_CLUSTER_FREE;
  // }
  // mark last
  if (FAT32_CLUSTER_IS_EOF(*fat)) {
    kprintf("mark last %d=>", (*fat));
    // fat32_set_fat(node, *fat, cluster_n_offset, FAT32_CLUSTER_FREE);
  }
  kprintf("nbytes %d \n", nbytes);

  // todo
  // e->last_modify_date=;
  // if (offset > e->file_size) {
  // if(e->file_size==0){
  e->file_size += nbytes;
  // }else{
  // e->file_size = e->file_size - offset + nbytes;
  // }
  //}//
  kprintf("offset %d e->file_size=%d cluster=%d\n", offset, e->file_size,
          e->start_file_cluster_low);

  fat32_set_entry(node, file_info->entry_cluster, file_info->entry_index, e);

  char test[512];
  kmemset(test, 0, 512);
  fat32_read(node, 0, 12, test);
  fat32_printf_buffer("read", test, 12);

  return total_bytes;
}

u32 is_file(u32 attr) {
  return (attr & FAT32_ATTR_READ_ONLY_FILE) == FAT32_ATTR_READ_ONLY_FILE ||
         (attr & FAT32_ATTR_HIDDENT_FILE) == FAT32_ATTR_HIDDENT_FILE ||
         (attr & FAT32_ATTR_SYSTEM_FILE) == FAT32_ATTR_SYSTEM_FILE ||
         (attr & FAT32_ATTR_LONG_FILE_NAME) == FAT32_ATTR_LONG_FILE_NAME ||
         (attr & FAT32_ATTR_ARCHIVE) == FAT32_ATTR_ARCHIVE ||
         (attr & FAT32_ATTR_DIRECTORY) == FAT32_ATTR_DIRECTORY;
}

u32 fat32_find_free_entry(vnode_t *node, u32 cluster) {
  u32 bytes = fat32_info->bytes_per_cluster;
  static u8 *entries = NULL;
  if (entries == NULL) {
    entries = kmalloc(bytes);
  }
  u32 entries_number = bytes / sizeof(dir_entry_t);
  fat32_t *fat = &cluster;
  u32 cluster_offset = 0;
  while (!FAT32_CLUSTER_IS_EOF(*fat) && *fat != 0) {
    // kprintf("fat %x\n",*fat);
    u32 read_offset = cluster_data(*fat);
    u32 ret = fat32_read_bytes(node, read_offset, bytes, entries);
    for (int i = 0; i < entries_number; i++) {
      dir_entry_t *e = &((dir_entry_t *)entries)[i];
      lfn_entry_t *lfn = e;
      // kprintf("%x\n",e->attr);
      if (e->attr == 0 || e->name[0] == 0xE5 || e->name[0] == 0) {
        return i;
      }
    }

    read_offset += bytes;
    fat = fat32_next_fat(node, *fat, &cluster_offset);
  }
  if (FAT32_CLUSTER_IS_EOF(*fat)) {
    kprintf("cannot find free entry\n");
  }
  return -1;
}

u32 fat32_set_entry(vnode_t *node, u32 cluster, u32 fat_index,
                    dir_entry_t *entry) {
  char buffer[512];
  kmemset(buffer, 0, 512);
  u32 read_offset = cluster_data(cluster);
  kprintf("read offset %d entry index %d\n", read_offset, fat_index);
  fat32_read_bytes(node, read_offset, 512, buffer);
  dir_entry_t *e = buffer;
  e[fat_index] = *entry;
  fat32_write_bytes(node, read_offset, 512, buffer);
  return 1;
}

u32 fat32_create_file(vnode_t *node) {
  file_info_t *parent_file_info = node->data;
  u32 cluster = parent_file_info->entry_cluster;
  file_info_t *file_info = kmalloc(sizeof(file_info_t));
  dir_entry_t *entry = kmalloc(sizeof(dir_entry_t));

  u32 fat_cluster = parent_file_info->fat_info->vbr->first_cluster - 2;
  u32 new_fat_index = fat32_find_free_fat(node, fat_cluster);
  kprintf("create file new fat index %d\n", new_fat_index);
  new_fat_index += 2;
  fat32_set_fat(node, fat_cluster, new_fat_index, FAT32_CLUSTER_EOF);

  entry->start_file_cluster_hight = new_fat_index & 0xff00 >> 16;
  entry->start_file_cluster_low = new_fat_index & 0xff;
  entry->attr = FAT32_ATTR_ARCHIVE;
  entry->file_size = 0;
  char *s = kstrchr(node->name, '.');
  if (s != NULL) {
    char buf[8];
    kmemset(buf, 0x20, 8);
    kstrncpy(buf, node->name, s - node->name);
    for (int i = 0; i < 8; i++) {
      buf[i] = ktoupper(buf[i]);
    }
    kstrncpy(entry->name, buf, 8);
    kstrncpy(entry->ext, s + 1, 3);
    for (int i = 0; i < 3; i++) {
      entry->ext[i] = ktoupper(entry->ext[i]);
    }
  } else {
    kstrncpy(entry->name, node->name, 8);
    entry->ext[0] = 32;
    entry->ext[1] = 32;
    entry->ext[2] = 32;
  }
  int new_entry_index = fat32_find_free_entry(node, cluster);
  if (new_entry_index < 0) {
    kprintf("find entry error\n");
    return -1;
  }
  file_info->entry_index = new_entry_index;
  file_info->entry_cluster = cluster;

  fat32_set_entry(node, cluster, new_entry_index, entry);
  file_info->entry = entry;
  file_info->entry_index = new_entry_index;
  fat32_init_op(node);
  node->data = file_info;
  return 1;
}

u32 fat32_open(vnode_t *node) {
  char *name = node->name;
  file_info_t *file_info = node->data;
  if (file_info->entry == NULL) {
    kprintf("create new file %s\n", name);
    fat32_create_file(node);
  }else{
    //read length
    file_info_t *file_info = node->data;
    node->length=file_info->entry->file_size;
    kprintf("open and get file size %d\n", node->length);
  }
  return 1;
}

vnode_t *fat32_find(vnode_t *node, char *name) {
  file_info_t *file_info = node->data;
  fat32_info_t *fat32 = file_info->fat_info;
  u32 index = 0;
  dir_entry_t *e = fat32_find_file_entry(fat32->entries, fat32->entries_number,
                                         name, &index);
  if (e == NULL) {
    kprintf("find file %s error\n", name);
    return NULL;
  }
  // kprintf("find cluster:%d\n",e->start_file_cluster_low);
  u32 type = V_FILE;
  if (e->attr == FAT32_ATTR_DIRECTORY) {
    type = V_DIRECTORY;
  }
  vnode_t *file = vfs_create_node(name, type);
  file_info_t *new_file_info = kmalloc(sizeof(file_info_t));
  new_file_info->entry = e;
  new_file_info->entry_index = index;
  file->data = new_file_info;
  file->device = node->device;
  fat32_init_op(file);
  return file;
}

vdirent_t *fat32_read_dir(vnode_t *node, u32 index) {
  if (!(node->flags == V_FILE || node->flags == V_DIRECTORY)) {
    kprintf("read dir failed for not file\n");
    return NULL;
  }
  vdirent_t *dirent = kmalloc(sizeof(vdirent_t));
  file_info_t *file = node->data;
  // read entry
  dir_entry_t *entry = file->entry;
  fat32_t cluster = cluster_no(entry);
  u32 cluster_offset = index;

  dir_entry_t entries[512];
  u32 entries_number = 512 / sizeof(fat32_t);

  char lnf_name[256];
  fat32_t *fat = &cluster;
  u32 read_offset = fat32_info->data + ((cluster)-2) *
                                           fat32_info->vbr->sector_per_cluster *
                                           fat32_info->vbr->byte_per_sector;

  u32 count = 0;
  while (!FAT32_CLUSTER_IS_EOF(*fat) && *fat != 0) {
    u32 ret = fat32_read_bytes(node, read_offset, 512, entries);
    if (index <= (count + entries_number)) {
      dir_entry_t *e = &entries[index];
      lfn_entry_t *lfn = e;
      if (is_file(e->attr) == 0) {
        return NULL;
      }
      if ((e->attr & FAT32_ATTR_DIRECTORY) == FAT32_ATTR_DIRECTORY) {
        dirent->type = V_DIRECTORY;
      } else if ((e->attr & FAT32_ATTR_ARCHIVE) == FAT32_ATTR_ARCHIVE) {
        dirent->type = V_FILE;
      }
      kmemset(dirent->name, 0, 128);
      if (e->attr != FAT32_ATTR_LONG_FILE_NAME) {
        kstrncpy(dirent->name, e->name, 11);
        // kprintf("%d name %s\n", index, e->name);
        return dirent;
      } else if ((lfn->seq & 0xc0) == 0x40) {
        int j = fat32_get_long_file_name(lfn, lnf_name);
        // kprintf("%d lnf_name %s\n", index, lnf_name);
        kstrcpy(dirent->name, lnf_name);
        return dirent;
      }
      read_offset += 512;
    }
    count += entries_number;
    fat = fat32_next_fat(node, *fat, &cluster_offset);
  }
  return NULL;
}

voperator_t fat32_op = {
    .read = fat32_read,
    .write = fat32_write,
    .open = fat32_open,
    .close = fat32_close,
    .find = fat32_find,
    .readdir = fat32_read_dir,
};

void fat32_init_op(vnode_t *node) { node->op = &fat32_op; }

void fat32_close(vnode_t *node) {}

int fat32_init(void) {
  vnode_t *node = vfs_find(NULL, "/dev/sda");
  if (node == NULL) {
    kprintf("not found sda\n");
  }
  fat32_init_op(node);

  file_info_t *file_info = kmalloc(sizeof(file_info_t));
  fat32_info = kmalloc(sizeof(fat32_info_t));
  file_info->fat_info = fat32_info;

  node->data = file_info;

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

  fat32_info->entries_number = 120;
  dir_entry_t *entries =
      kmalloc(sizeof(dir_entry_t) * fat32_info->entries_number);
  fat32_read_bytes(node, data, sizeof(dir_entry_t) * fat32_info->entries_number,
                   entries);
  fat32_info->entries = entries;

  fat32_info->bytes_per_cluster =
      fat32_info->vbr->sector_per_cluster * fat32_info->vbr->byte_per_sector;

  file_info->entry_cluster = fat32_info->vbr->first_cluster;
  return 0;
}

void fat32_exit(void) { kprintf("fat32 exit\n"); }

module_t fat32_module = {
    .name = "fat32", .init = fat32_init, .exit = fat32_exit};