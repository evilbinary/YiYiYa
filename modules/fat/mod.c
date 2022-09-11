#include "fat.h"
#include "fat_config.h"
#include "kernel/device.h"
#include "kernel/memory.h"
#include "kernel/stat.h"
#include "rtc/rtc.h"

#ifdef ARM
#include "mmc/sdhci.h"
#else
#include "ahci/ahci.h"
#endif

enum {
  DT_UNKNOWN = 0,
#define DT_UNKNOWN DT_UNKNOWN
  DT_FIFO = 1,
#define DT_FIFO DT_FIFO
  DT_CHR = 2,
#define DT_CHR DT_CHR
  DT_DIR = 4,
#define DT_DIR DT_DIR
  DT_BLK = 6,
#define DT_BLK DT_BLK
  DT_REG = 8,
#define DT_REG DT_REG
  DT_LNK = 10,
#define DT_LNK DT_LNK
  DT_SOCK = 12,
#define DT_SOCK DT_SOCK
  DT_WHT = 14
#define DT_WHT DT_WHT
};
typedef struct file_info {
  struct fat_fs_struct *fs;
  struct fat_dir_struct *dd;
  struct fat_file_struct *fd;
  int offset;
} file_info_t;

typedef uint8_t (*sd_raw_read_interval_handler_t)(uint8_t *buffer,
                                                  offset_t offset, void *p);
typedef uintptr_t (*sd_raw_write_interval_handler_t)(uint8_t *buffer,
                                                     offset_t offset, void *p);

vnode_t *default_node = NULL;

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
  u32 ret = 0;
  u32 count = nbytes / BYTE_PER_SECTOR;
  u32 rest = nbytes % BYTE_PER_SECTOR;
  char small_buf[BYTE_PER_SECTOR*2];
  for (int i = 0; i < count; i++) {
    kmemset(small_buf, 0, BYTE_PER_SECTOR*2);
    ret = fat_device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    kmemmove(buf, small_buf + offset % BYTE_PER_SECTOR, BYTE_PER_SECTOR);
    buf += BYTE_PER_SECTOR;
    offset += BYTE_PER_SECTOR;
  }
  if (rest > 0) {
    kmemset(small_buf, 0, BYTE_PER_SECTOR*2);
    ret = fat_device_read(node, offset, BYTE_PER_SECTOR*2, small_buf);
    buf += count * BYTE_PER_SECTOR;
    kmemmove(buf, small_buf + offset % BYTE_PER_SECTOR, rest);
  }
  if (ret < 0) {
    kprintf("fat read bytes error\n");
    return -1;
  }
  return nbytes;
}

size_t fat_write_bytes(vnode_t *node, u32 offset, size_t nbytes, u8 *buf) {
  u32 count = nbytes / BYTE_PER_SECTOR;
  u32 rest = nbytes % BYTE_PER_SECTOR;
  char small_buf[BYTE_PER_SECTOR * 2];
  u32 ret = 0;
  for (int i = 0; i < count; i++) {
    kmemset(small_buf, 0, BYTE_PER_SECTOR);
    fat_device_read(node, offset, BYTE_PER_SECTOR, small_buf);
    kmemmove(small_buf + offset % BYTE_PER_SECTOR, buf, BYTE_PER_SECTOR);
    ret = fat_device_write(node, offset, BYTE_PER_SECTOR, small_buf);
    buf += BYTE_PER_SECTOR;
    offset += BYTE_PER_SECTOR;
  }
  if (rest > 0) {
    kmemset(small_buf, 0, BYTE_PER_SECTOR);
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
  if (file_info == NULL) {
    kprintf("write file info faild not opend\n");
    return -1;
  }
  if (file_info->fd == NULL) {
    kprintf("write error fd null\n");
    return -1;
  }
  fat_seek_file(file_info->fd, &offset, FAT_SEEK_SET);
  u32 ret=fat_write_file(file_info->fd, buffer, nbytes);
  if(file_info->fd!=NULL){
    node->length=file_info->fd->dir_entry.file_size;
  }
  return ret;
}

u32 find_file_in_dir(struct fat_fs_struct *fs, struct fat_dir_struct *dd,
                     const char *name, struct fat_dir_entry_struct *dir_entry) {
  while (fat_read_dir(dd, dir_entry)) {
    if (kstrcmp(dir_entry->long_name, name) == 0) {
      // kprintf("find_file_in_dir %s==%s
      // attr:%x\n",dir_entry->long_name,name,dir_entry->attributes);
      fat_reset_dir(dd);
      return 1;
    }
  }

  return 0;
}

int open_file_in_dir(struct fat_fs_struct *fs, struct fat_dir_struct *dd,
                     const char *name, file_info_t *new_file_info) {
  struct fat_dir_entry_struct file_entry;
  if (kstrlen(name) == 0) {
    new_file_info->dd = dd;
    return 1;
  }
  u32 ret = find_file_in_dir(fs, dd, name, &file_entry);
  if (!ret) {
    kprintf("find_file_in_dir failed %s\n", name);
    return 0;
  }
  if (file_entry.attributes & FAT_ATTRIB_DIR) {
    new_file_info->dd = fat_open_dir(fs, &file_entry);
    new_file_info->fd = NULL;
  } else {
    new_file_info->dd = NULL;
    new_file_info->fd = fat_open_file(fs, &file_entry);
  }
  return 1;
}

u32 fat_op_open(vnode_t *node, u32 mode) {
  char *name = node->name;
  struct fat_dir_entry_struct directory;
  file_info_t *file_info = node->data;
  if (file_info == NULL) {
    file_info = node->super->data;
    node->data = file_info;
  }
  struct fat_fs_struct *fs = file_info->fs;
  file_info->offset = 0;
  if (((mode & O_CREAT) == O_CREAT) ||
      (file_info->fd == NULL && file_info->dd == NULL)) {
    kprintf("create new file %s\n", name);
    file_info_t *parent_file_info = node->parent->data;
    file_info_t *new_file_info = kmalloc(sizeof(struct file_info));
    struct fat_dir_struct *dd = kmalloc(sizeof(struct fat_dir_struct));
    new_file_info->dd = dd;
    dd->fs = fs;
    node->data = new_file_info;
    uint8_t res = fat_create_file(parent_file_info->dd, name, &dd->dir_entry);
    if (!res) {
      kprintf("fat_create_file faild\n");
      return -1;
    }
    struct fat_file_struct *fd = fat_open_file(fs, &dd->dir_entry);
    if (!fd) {
      kprintf("create file bad fd %s\n", name);
      return -1;
    }
    new_file_info->fd = fd;
  }else{
    if(file_info->fd!=NULL){
      node->length =file_info->fd->dir_entry.file_size;
    }
  }
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

  file_info_t *new_file_info = kmalloc(sizeof(file_info_t));
  new_file_info->fs = fs;
  int ret = open_file_in_dir(fs, dd, name, new_file_info);
  if (!ret) {
    kprintf("bad fd %s\n", name);
    return NULL;
  }

  u32 type = V_FILE;
  if (dd->dir_entry.attributes == FAT_ATTRIB_DIR) {
    type = V_DIRECTORY;
  }
  vnode_t *file = vfs_create_node(name, type);
  file->data = new_file_info;
  if(new_file_info->fd!=NULL){
      file->length =new_file_info->fd->dir_entry.file_size;
  }
  file->device = node->device;
  fat_init_op(file);
  return file;
}

u32 fat_op_read_dir(vnode_t *node, struct vdirent *dirent, u32 count) {
  if (!((node->flags & V_FILE) == V_FILE ||
        (node->flags & V_DIRECTORY) == V_DIRECTORY)) {
    kprintf("read dir failed for not file flags is %x\n", node->flags);
    return 0;
  }
  file_info_t *file_info = node->data;
  struct fat_dir_entry_struct dir_entry;
  u32 i = 0;
  u32 nbytes = 0;
  u32 total = 0;
  while (fat_read_dir(file_info->dd, &dir_entry)) {
    total++;
  }
  while (fat_read_dir(file_info->dd, &dir_entry)) {
    if (file_info->offset >= total) {
      return nbytes;
    }
    if (i < count) {
      if ((dir_entry.attributes & FAT_ATTRIB_DIR) == FAT_ATTRIB_DIR) {
        dirent->type = DT_DIR;
      } else if ((dir_entry.attributes & FAT_ATTRIB_ARCHIVE) ==
                 FAT_ATTRIB_ARCHIVE) {
        dirent->type = DT_REG;
      }
      kstrncpy(dirent->name, dir_entry.long_name, 256);
      dirent->offset = i;
      dirent->length = sizeof(struct vdirent);
      nbytes += dirent->length;
      dirent++;
      file_info->offset++;
    }
    i++;
  }

  return nbytes;
}

int fat_op_close(vnode_t *node) {
  file_info_t *file_info = node->data;
  if (file_info != NULL) {
    file_info->offset = 0;
    fat_close_file(file_info->fd);
  }
  return 0;
}

size_t fat_op_ioctl(struct vnode *node, u32 cmd, void *args){
  u32 ret = 0;

  if(cmd==IOC_STAT){
    file_info_t *file_info = node->data;
    if (file_info == NULL) {
      file_info = node->super->data;
      node->data = file_info;
    }
    struct stat *stat=args;
    if(file_info->dd!=NULL){
      stat->st_size= file_info->dd->dir_entry.file_size;
      stat->st_mtim= file_info->dd->dir_entry.modification_time;
      stat->st_mode= file_info->dd->dir_entry.attributes;
    }else if(file_info->fd!=NULL){
      stat->st_size= file_info->fd->dir_entry.file_size;
      stat->st_mtim= file_info->fd->dir_entry.modification_time;
      stat->st_mode= file_info->fd->dir_entry.attributes;
    }

    return 0;
  }

  device_t *dev = node->device;
  if (dev == NULL) {
    return ret;
  }
  // va_list args;
  // va_start(args, cmd);
  ret = dev->ioctl(dev, cmd, args);
  // va_end(args);
  return ret;
}


void get_datetime(uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* min, uint8_t* sec){
  int time_fd=-1;
  time_fd = sys_open("/dev/time", 0);
  if (time_fd < 0) return ;

   rtc_time_t time;
  time.day = 1;
  time.hour = 0;
  time.minute = 0;
  time.month = 1;
  time.second = 0;
  time.year = 1900;
  int ret = sys_read(time_fd, &time, sizeof(rtc_time_t));
  if (ret < 0) {
    kprintf("erro read time\n");
    return ;
  }
  *year=time.year;
  *month=time.month;
  *day=time.day;
  *hour=time.hour;
  *min=time.minute;
  *sec=time.second;
}

voperator_t fat_op = {
    .read = fat_op_read,
    .write = fat_op_write,
    .open = fat_op_open,
    .close = fat_op_close,
    .find = fat_op_find,
    .readdir = fat_op_read_dir,
    .ioctl = fat_op_ioctl,
};

void fat_init_op(vnode_t *node) { node->op = &fat_op; }

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
  u32 res;
  res = fat_get_dir_entry_of_path(fs, "/", &directory);

  struct fat_dir_struct *dd = fat_open_dir(fs, &directory);
  if (!dd) {
    kprintf("bad dd\n");
    return NULL;
  }

  file_info_t *file_info = kmalloc(sizeof(file_info_t));
  file_info->fs = fs;
  file_info->dd = dd;
  node->data = file_info;
  // todo why malloc?
  kmalloc(1024 * 4);
  kprintf("fat init end\n");
}

void fat_exit(void) { kprintf("fat exit\n"); }

module_t fat_module = {.name = "fat", .init = fat_init, .exit = fat_exit};