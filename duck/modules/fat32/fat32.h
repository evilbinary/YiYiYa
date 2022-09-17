/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef FAT32_H
#define FAT32_H

#ifdef ARM
#include "mmc/sdhci.h"
#else
#include "ahci/ahci.h"
#endif
#include "kernel/device.h"
#include "kernel/kernel.h"
#include "kernel/module.h"
#include "kernel/vfs.h"

#define FAT32_CLUSTER_IS_EOF(c) ((c) == 0xFFFFFF8 || (c) == 0xFFFFFF9 || (c) == 0xFFFFFFA || \
                                 (c) == 0xFFFFFFB || (c) == 0xFFFFFFC || (c) == 0xFFFFFFD || \
                                 (c) == 0xFFFFFFE || (c) == 0xFFFFFFF)

#define FAT32_CLUSTER_EOF 0xFFFFFFE
#define FAT32_ENTRY_MASK  0x0FFFFFFF
#define FAT32_CLUSTER_FREE 0

#define FAT32_ATTR_READ_ONLY_FILE 0x01
#define FAT32_ATTR_HIDDENT_FILE 0x02
#define FAT32_ATTR_SYSTEM_FILE 0x04
#define FAT32_ATTR_VOLUME_LABEL 0x08
#define FAT32_ATTR_LONG_FILE_NAME 0x0f
#define FAT32_ATTR_DIRECTORY 0x10
#define FAT32_ATTR_ARCHIVE 0x20

#define FAT32_ATTR_MASK		0x3F

#define cluster_no(entry) ((entry->start_file_cluster_hight << 16) | entry->start_file_cluster_low)
#define cluster_fat(cluster) (fat32_info->fat1 + sizeof(fat32_t) * (cluster))
#define cluster_data(cluster) (fat32_info->data +(cluster - 2) * fat32_info->vbr->sector_per_cluster *fat32_info->vbr->byte_per_sector)

typedef struct vbr {
  u8 jmp[3];
  u8 oem_name[8];
  u16 byte_per_sector;        //每扇区字节数
  u8 sector_per_cluster;      //每簇扇区数
  u16 reserved_sector_count;  //保留扇区数
  u8 fat_number;              // FAT表个数
  u16 root_dir_number;  // FAT32必须等于0，FAT12/FAT16为根目录中目录的个数；
  u16 sector_number;  // FAT32必须等于0，FAT12/FAT16为扇区总数。
  u8 media;  //哪种存储介质，0xF8标准值，可移动存储介质。
  u16 fat_sector_number;  // FAT32必须为0，FAT12/FAT16为一个FAT 表所占的扇区数。
  u16 sector_per_track;  //每磁道扇区数，只对于“特殊形状”（由磁头和柱面分割为若干磁道）的存储介质有效，0x003F=63。
  u16 header_number;     //磁头数(面数)
  u32 hidden_sector_number;   //隐藏扇区数
  u32 total_sector_number;    //文件系统总扇区数
  u32 sector_per_fat_number;  //每个FAT表占用扇区数
  u16 ext_flags;              //标记，此域FAT32 特有
  u16 version;                // FAT32版本号0.0，FAT32特有。
  u32 first_cluster;          //根目录所在第一个簇的簇号
  u16 fs_info;  // FSINFO（文件系统信息扇区）扇区号0x01，该扇区为操作系统提供关于空簇总数及下一可用簇的信息。
  u16 backup_sector;  //备份引导扇区的位置
  u8 reserved[12];    // 12字节，用于以后FAT 扩展使用。
  u8 drive_number;
  u8 reserved1;   // 1字节，与FAT12/16 的定义相同
  u8 boot_sig;    //扩展引导标志，0x29
  u32 volumn_no;  // 4字节，卷序列号。通常为一个随机值。
  u8 volumn[11];  //卷标（ASCII码）
  u8 file_system_type[8];  //文件系统格式的ASCII码，FAT32

} __attribute__((packed)) vbr_t;

typedef struct fs_info {
  u32 sector_signature;                //扩展引导标志“0x52526141”
  u8 reserved[480];                    //未使用，全部置0
  u32 fs_sector_signature;             // FSINFO签名“0x72724161”。
  u32 last_know_number_free_clusters;  // Last known number of free data
                                       // clusters on the volume
  u32 recently_clusters_number;
  u8 reserved2[12];
  u16 fs_info_sector_signature;  // FS information sector signature 0x00 0x00
                                 // 0x55 0xAA
} __attribute__((packed)) fs_info_t;

typedef u32 fat32_t;

typedef struct dir_entry {
  u8 name[8];
  u8 ext[3];
  u8 attr;
  u8 user_attr;
  u8 first_character;
  u16 create_time;
  u16 create_date;
  u16 last_access_date;
  u16 start_file_cluster_hight;  //文件起始簇号的高16位 0x0000
  u16 last_modify_time;      //文件最近修改时间
  u16 last_modify_date;      //文件最近修改日期
  u16 start_file_cluster_low;    //文件起始簇号的地16位 0x0003
  u32 file_size;             //文件的长度bytes
} __attribute__((packed)) dir_entry_t;

typedef struct lfn_entry{
  u8 seq;
  u8 name1[10];
  u8 attr;
  u8 reserved;
  u8 checksum;
  u8 name2[12];
  u16 reserved1;
  u8 name3[4];
}__attribute__((packed)) lfn_entry_t;


typedef struct fat32_info{
    vbr_t* vbr;
    dir_entry_t* entries;
    u32 entries_number;
    fs_info_t* fs_info;
    u32 fat1;
    u32 fat2;
    u32 root_dir;
    u32 data;
    u32 fat_size;
    u32 bytes_per_cluster;
}fat32_info_t;

typedef struct file_info{
  fat32_info_t* fat_info;
  u32 entry_cluster;
  u32 entry_index;
  dir_entry_t* entry;
}file_info_t;

void fat32_init_op(vnode_t* node);
int fat32_init(void);
void fat32_close(vnode_t *node);
vnode_t *fat32_find(vnode_t *node, char *name);
u32 fat32_write(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer);
u32 fat32_read(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer);

size_t fat32_read_bytes(vnode_t *node, u32 offset, size_t nbytes, u8 *buf);
size_t fat32_write_bytes(vnode_t *node, u32 offset, size_t nbytes, u8 *buf);

u32 fat32_set_entry(vnode_t *node, u32 cluster, u32 fat_index,
                    dir_entry_t *entry) ;

#endif