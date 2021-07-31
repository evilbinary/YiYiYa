#include "fat.h"
#include "fat_config.h"
#include "kernel/device.h"

#ifdef ARM
#include "drivers/mmc/sdhci.h"
#else
#include "drivers/ahci/ahci.h"
#endif

typedef struct file_info {
  struct fat_fs_struct *fs;
  struct partition_struct *partition;
  struct fat_dir_struct *dd;
  struct fat_file_struct *fd;
} file_info_t;

typedef uint8_t (*sd_raw_read_interval_handler_t)(uint8_t *buffer,
                                                  offset_t offset, void *p);
typedef uintptr_t (*sd_raw_write_interval_handler_t)(uint8_t *buffer,
                                                     offset_t offset, void *p);

vnode_t *default_node;

static u32 fat_device_read(vnode_t *node, u32 offset, size_t nbytes,
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

static u32 fat_device_write(vnode_t *node, u32 offset, size_t nbytes,
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

size_t fat_read_bytes(vnode_t *node, u32 offset, size_t nbytes, u8 *buf) {
  u32 count = nbytes / BYTE_PER_SECTOR;
  u32 rest = nbytes % BYTE_PER_SECTOR;
  char small_buf[BYTE_PER_SECTOR];
  u32 ret = 0;
  for (int i = 0; i < count; i++) {
    ret = fat_device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    kmemmove(buf, small_buf + offset % BYTE_PER_SECTOR, BYTE_PER_SECTOR);
    buf += BYTE_PER_SECTOR;
    offset += BYTE_PER_SECTOR;
  }
  if (rest > 0) {
    ret = fat_device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    buf += count * BYTE_PER_SECTOR;
    kmemmove(buf, small_buf + offset % BYTE_PER_SECTOR, rest);
  }
  if (ret < 0) {
    return -1;
  }
  return nbytes;
}

size_t fat_write_bytes(vnode_t *node, u32 offset, size_t nbytes, u8 *buf) {
  u32 count = nbytes / BYTE_PER_SECTOR;
  u32 rest = nbytes % BYTE_PER_SECTOR;
  char small_buf[BYTE_PER_SECTOR * 2];
  kmemset(small_buf, 0, BYTE_PER_SECTOR);
  u32 ret = 0;
  for (int i = 0; i < count; i++) {
    fat_device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    kmemmove(small_buf + offset % BYTE_PER_SECTOR, buf, BYTE_PER_SECTOR);
    ret = fat_device_write(node, offset, BYTE_PER_SECTOR, small_buf);
    buf += BYTE_PER_SECTOR;
    offset += BYTE_PER_SECTOR;
  }
  if (rest > 0) {
    fat_device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    kmemmove(small_buf + offset % BYTE_PER_SECTOR, buf, rest);
    ret = fat_device_write(node, offset, BYTE_PER_SECTOR, small_buf);
    buf += count * BYTE_PER_SECTOR;
  }
  if (ret < 0) {
    return -1;
  }
  return nbytes;
};

uint8_t sd_raw_read(offset_t offset, uint8_t *buffer, uintptr_t length) {
  // kprintf(" sd_raw_read %x buffer %x len %d\n",offset,buffer,length);
  fat_read_bytes(default_node, offset, length, buffer);
  return 1;
}

uint8_t sd_raw_write(offset_t offset, const uint8_t *buffer, uintptr_t length) {
  fat_write_bytes(default_node, offset, length, buffer);
  return 1;
}

uint8_t sd_raw_read_interval(offset_t offset, uint8_t *buffer,
                             uintptr_t interval, uintptr_t length,
                             sd_raw_read_interval_handler_t callback, void *p) {
  if (!buffer || interval == 0 || length < interval || !callback) return 0;
  while (length >= interval) {
    if (!sd_raw_read(offset, buffer, interval)) return 0;
    if (!callback(buffer, offset, p)) break;
    offset += interval;
    length -= interval;
  }

  return 1;
}

uint8_t sd_raw_write_interval(offset_t offset, uint8_t *buffer,
                              uintptr_t length,
                              sd_raw_write_interval_handler_t callback,
                              void *p) {
  if (!buffer || !callback) return 0;

  uint8_t endless = (length == 0);
  while (endless || length > 0) {
    uint16_t bytes_to_write = callback(buffer, offset, p);
    if (!bytes_to_write) break;
    if (!endless && bytes_to_write > length) return 0;
    if (!sd_raw_write(offset, buffer, bytes_to_write)) return 0;
    offset += bytes_to_write;
    length -= bytes_to_write;
  }

  return 1;
}

u32 fat_op_read(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {
  file_info_t *file_info = node->data;
  if (file_info->fd == NULL) {
    kprintf("read error fd null\n");
    return -1;
  }
  fat_seek_file(file_info->fd, &offset, FAT_SEEK_SET);
  return fat_read_file(file_info->fd, buffer, nbytes);
}

u32 fat_op_write(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {
  file_info_t *file_info = node->data;
  if (file_info->fd == NULL) {
    kprintf("write error fd null\n");
    return -1;
  }
  fat_seek_file(file_info->fd, &offset, FAT_SEEK_SET);
  return fat_write_file(file_info->fd, buffer, nbytes);
}

uint8_t find_file_in_dir(struct fat_fs_struct *fs, struct fat_dir_struct *dd,
                         const char *name,
                         struct fat_dir_entry_struct *dir_entry) {
  while (fat_read_dir(dd, dir_entry)) {
    if (kstrcmp(dir_entry->long_name, name) == 0) {
      fat_reset_dir(dd);
      return 1;
    }
  }

  return 0;
}

struct fat_file_struct *open_file_in_dir(struct fat_fs_struct *fs,
                                         struct fat_dir_struct *dd,
                                         const char *name) {
  struct fat_dir_entry_struct file_entry;
  if (!find_file_in_dir(fs, dd, name, &file_entry)) return 0;

  return fat_open_file(fs, &file_entry);
}

u32 fat_op_open(vnode_t *node) {
  char *name = node->name;
  struct fat_dir_entry_struct directory;
  file_info_t *file_info = node->data;
  struct fat_fs_struct *fs = file_info->fs;

  if (file_info->fd == NULL) {
    kprintf("create new file %s\n", name);
    file_info_t *parent_file_info=node->parent->data;
    file_info_t *new_file_info = kmalloc(sizeof(file_info_t));
    struct fat_dir_struct* dd=kmalloc(sizeof(struct fat_dir_struct));
    new_file_info->dd=dd;
    dd->fs=fs;
    node->data=new_file_info;
    uint8_t res=fat_create_file(parent_file_info->dd,name,&dd->dir_entry);
    if(!res){
      kprintf("fat_create_file faild\n");
      return -1;
    }
    struct fat_file_struct *fd = fat_open_file(fs, &dd->dir_entry);
    if (!fd) {
      kprintf("create file bad fd %s\n", name);
      return -1;
    }
    new_file_info->fd=fd;
  }
  // uint8_t res;
  // res = fat_get_dir_entry_of_path(fs, name, &directory);
  // if (!res) {
  //   kprintf("bad direc");
  //   return -1;
  // }

  // struct fat_dir_struct *dd = fat_open_dir(fs, &directory);
  // if (!dd) {
  //   kprintf("bad dd");
  //   return -1;
  // }
  // file_info->dd = dd;

  return 1;
}

vnode_t *fat_op_find(vnode_t *node, char *name) {
  file_info_t *file_info = node->data;
  struct fat_fs_struct *fs = file_info->fs;
  struct fat_dir_entry_struct directory;
  uint8_t res;
  res = fat_get_dir_entry_of_path(fs, "/", &directory);
  if (!res) {
    kprintf("bad direc /\n");
    return NULL;
  }
  struct fat_dir_struct *dd = fat_open_dir(fs, &directory);
  if (!dd) {
    kprintf("bad dd\n");
    return NULL;
  }

  struct fat_file_struct *fd = open_file_in_dir(fs, dd, name);
  if (!fd) {
    kprintf("bad fd %s\n", name);
    return NULL;
  }

  u32 type = V_FILE;
  if (dd->dir_entry.attributes == FAT_ATTRIB_DIR) {
    type = V_DIRECTORY;
  }
  vnode_t *file = vfs_create(name, type);
  file_info_t *new_file_info = kmalloc(sizeof(file_info_t));
  new_file_info->dd = dd;
  new_file_info->fd = fd;
  new_file_info->fs = fs;

  file->data = new_file_info;
  file->device = node->device;
  fat_init_op(file);
  return file;
}

vdirent_t *fat_op_read_dir(vnode_t *node, u32 index) {
  if (!(node->flags == V_FILE || node->flags == V_DIRECTORY)) {
    kprintf("read dir failed for not file\n");
    return NULL;
  }
  file_info_t *file_info = node->data;
  vdirent_t *dirent = kmalloc(sizeof(vdirent_t));
  struct fat_dir_entry_struct dir_entry;
  u32 i = 0;
  while (fat_read_dir(file_info->dd, &dir_entry)) {
    if (i == index) {
      if ((dir_entry.attributes & FAT_ATTRIB_DIR) == FAT_ATTRIB_DIR) {
        dirent->type = V_DIRECTORY;
      } else if ((dir_entry.attributes & FAT_ATTRIB_ARCHIVE) == FAT_ATTRIB_ARCHIVE) {
        dirent->type = V_FILE;
      }
      kstrcpy(dirent->name,dir_entry.long_name);
      return dirent;
    }
    i++;
  }
  return NULL;
}

void fat_op_close(vnode_t *node) {
  file_info_t *file_info = node->data;
  fat_close_file(file_info->fd);
}

void fat_init_op(vnode_t *node) {
  node->read = fat_op_read;
  node->write = fat_op_write;
  node->open = fat_op_open;
  node->close = fat_op_close;
  node->find = fat_op_find;
  node->readdir = fat_op_read_dir;
}

void fat_init(void) {
  kprintf("fat init\n");
  vnode_t *node = vfs_find(NULL, "/dev/sda");
  default_node = node;
  if (node == NULL) {
    kprintf("not found sda\n");
  }
  fat_init_op(node);
  struct partition_struct *partition =
      partition_open(sd_raw_read, sd_raw_read_interval, sd_raw_write,
                     sd_raw_write_interval, -1);

  if (!partition) {
    kprintf("open partition error\n");
    return;
  }
  struct fat_fs_struct *fs = fat_open(partition);
  if (!fs) {
    kprintf("bad fs\n");
    return;
  }

  struct fat_dir_entry_struct directory;
  uint8_t res;
  res = fat_get_dir_entry_of_path(fs, "/", &directory);

  struct fat_dir_struct *dd = fat_open_dir(fs, &directory);
  if (!dd) {
    kprintf("bad dd\n");
    return NULL;
  }

  file_info_t *file_info = kmalloc(sizeof(file_info_t));
  file_info->fs = fs;
  file_info->dd =dd;
  node->data = file_info;
}

void fat_exit(void) { kprintf("fat exit\n"); }

module_t fat_module = {.name = "fat", .init = fat_init, .exit = fat_exit};