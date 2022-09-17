
/*
 * Copyright (c) 2006-2012 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef FAT_H
#define FAT_H

#include "fat_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup fat
 *
 * @{
 */
/**
 * \file
 * FAT header (license: GPLv2 or LGPLv2.1)
 *
 * \author Roland Riegel
 */

/**
 * \addtogroup fat_file
 * @{
 */

/** The file is read-only. */
#define FAT_ATTRIB_READONLY (1 << 0)
/** The file is hidden. */
#define FAT_ATTRIB_HIDDEN (1 << 1)
/** The file is a system file. */
#define FAT_ATTRIB_SYSTEM (1 << 2)
/** The file is empty and has the volume label as its name. */
#define FAT_ATTRIB_VOLUME (1 << 3)
/** The file is a directory. */
#define FAT_ATTRIB_DIR (1 << 4)
/** The file has to be archived. */
#define FAT_ATTRIB_ARCHIVE (1 << 5)

/** The given offset is relative to the beginning of the file. */
#define FAT_SEEK_SET 0
/** The given offset is relative to the current read/write position. */
#define FAT_SEEK_CUR 1
/** The given offset is relative to the end of the file. */
#define FAT_SEEK_END 2

/**
 * \ingroup fat_file
 * Describes a directory entry.
 */
struct fat_dir_entry_struct {
  /** The file's name, truncated to 31 characters. */
  char long_name[32];
  /** The file's attributes. Mask of the FAT_ATTRIB_* constants. */
  uint8_t attributes;
#if FAT_DATETIME_SUPPORT
  /** Compressed representation of modification time. */
  uint16_t modification_time;
  /** Compressed representation of modification date. */
  uint16_t modification_date;
#endif
  /** The cluster in which the file's first byte resides. */
  cluster_t cluster;
  /** The file's size. */
  uint32_t file_size;
  /** The total disk offset of this directory entry. */
  offset_t entry_offset;
};

/**
 * @}
 */
struct partition_struct;
struct fat_fs_struct;
struct fat_file_struct;
struct fat_dir_struct;

/**
 * \addtogroup fat FAT support
 *
 * This module implements FAT16/FAT32 read and write access.
 *
 * The following features are supported:
 * - File names up to 31 characters long.
 * - Unlimited depth of subdirectories.
 * - Short 8.3 and long filenames.
 * - Creating and deleting files.
 * - Reading and writing from and to files.
 * - File resizing.
 * - File sizes of up to 4 gigabytes.
 *
 * @{
 */
/**
 * \file
 * FAT implementation (license: GPLv2 or LGPLv2.1)
 *
 * \author Roland Riegel
 */

/**
 * \addtogroup fat_config FAT configuration
 * Preprocessor defines to configure the FAT implementation.
 */

/**
 * \addtogroup fat_fs FAT access
 * Basic functions for handling a FAT filesystem.
 */

/**
 * \addtogroup fat_file FAT file functions
 * Functions for managing files.
 */

/**
 * \addtogroup fat_dir FAT directory functions
 * Functions for managing directories.
 */

/**
 * @}
 */

#define FAT16_CLUSTER_FREE 0x0000
#define FAT16_CLUSTER_RESERVED_MIN 0xfff0
#define FAT16_CLUSTER_RESERVED_MAX 0xfff6
#define FAT16_CLUSTER_BAD 0xfff7
#define FAT16_CLUSTER_LAST_MIN 0xfff8
#define FAT16_CLUSTER_LAST_MAX 0xffff

#define FAT32_CLUSTER_FREE 0x00000000
#define FAT32_CLUSTER_RESERVED_MIN 0x0ffffff0
#define FAT32_CLUSTER_RESERVED_MAX 0x0ffffff6
#define FAT32_CLUSTER_BAD 0x0ffffff7
#define FAT32_CLUSTER_LAST_MIN 0x0ffffff8
#define FAT32_CLUSTER_LAST_MAX 0x0fffffff

#define FAT_DIRENTRY_DELETED 0xe5
#define FAT_DIRENTRY_LFNLAST (1 << 6)
#define FAT_DIRENTRY_LFNSEQMASK ((1 << 6) - 1)

/* Each entry within the directory table has a size of 32 bytes
 * and either contains a 8.3 DOS-style file name or a part of a
 * long file name, which may consist of several directory table
 * entries at once.
 *
 * multi-byte integer values are stored little-endian!
 *
 * 8.3 file name entry:
 * ====================
 * offset  length  description
 *      0       8  name (space padded)
 *      8       3  extension (space padded)
 *     11       1  attributes (FAT_ATTRIB_*)
 *
 * long file name (lfn) entry ordering for a single file name:
 * ===========================================================
 * LFN entry n
 *     ...
 * LFN entry 2
 * LFN entry 1
 * 8.3 entry (see above)
 *
 * lfn entry:
 * ==========
 * offset  length  description
 *      0       1  ordinal field
 *      1       2  unicode character 1
 *      3       3  unicode character 2
 *      5       3  unicode character 3
 *      7       3  unicode character 4
 *      9       3  unicode character 5
 *     11       1  attribute (always 0x0f)
 *     12       1  type (reserved, always 0)
 *     13       1  checksum
 *     14       2  unicode character 6
 *     16       2  unicode character 7
 *     18       2  unicode character 8
 *     20       2  unicode character 9
 *     22       2  unicode character 10
 *     24       2  unicode character 11
 *     26       2  cluster (unused, always 0)
 *     28       2  unicode character 12
 *     30       2  unicode character 13
 *
 * The ordinal field contains a descending number, from n to 1.
 * For the n'th lfn entry the ordinal field is or'ed with 0x40.
 * For deleted lfn entries, the ordinal field is set to 0xe5.
 */

struct fat_header_struct {
  offset_t size;

  offset_t fat_offset;
  uint32_t fat_size;

  uint16_t sector_size;
  uint16_t cluster_size;

  offset_t cluster_zero_offset;

  offset_t root_dir_offset;
#if FAT_FAT32_SUPPORT
  cluster_t root_dir_cluster;
#endif
};

struct fat_fs_struct {
  struct partition_struct* partition;
  struct fat_header_struct header;
  cluster_t cluster_free;
};

struct fat_file_struct {
  struct fat_fs_struct* fs;
  struct fat_dir_entry_struct dir_entry;
  offset_t pos;
  cluster_t pos_cluster;
};

struct fat_dir_struct {
  struct fat_fs_struct* fs;
  struct fat_dir_entry_struct dir_entry;
  cluster_t entry_cluster;
  uint16_t entry_offset;
};

struct fat_read_dir_callback_arg {
  struct fat_dir_entry_struct* dir_entry;
  uintptr_t bytes_read;
#if FAT_LFN_SUPPORT
  uint8_t checksum;
#endif
  uint8_t finished;
};

struct fat_usage_count_callback_arg {
  cluster_t cluster_count;
  uintptr_t buffer_size;
};

struct fat_fs_struct* fat_open(struct partition_struct* partition);
void fat_close(struct fat_fs_struct* fs);

struct fat_file_struct* fat_open_file(
    struct fat_fs_struct* fs, const struct fat_dir_entry_struct* dir_entry);
void fat_close_file(struct fat_file_struct* fd);
intptr_t fat_read_file(struct fat_file_struct* fd, uint8_t* buffer,
                       uintptr_t buffer_len);
intptr_t fat_write_file(struct fat_file_struct* fd, const uint8_t* buffer,
                        uintptr_t buffer_len);
uint8_t fat_seek_file(struct fat_file_struct* fd, int32_t* offset,
                      uint8_t whence);
uint8_t fat_resize_file(struct fat_file_struct* fd, uint32_t size);

struct fat_dir_struct* fat_open_dir(
    struct fat_fs_struct* fs, const struct fat_dir_entry_struct* dir_entry);
void fat_close_dir(struct fat_dir_struct* dd);
uint8_t fat_read_dir(struct fat_dir_struct* dd,
                     struct fat_dir_entry_struct* dir_entry);
uint8_t fat_reset_dir(struct fat_dir_struct* dd);

uint8_t fat_create_file(struct fat_dir_struct* parent, const char* file,
                        struct fat_dir_entry_struct* dir_entry);
uint8_t fat_delete_file(struct fat_fs_struct* fs,
                        struct fat_dir_entry_struct* dir_entry);
uint8_t fat_move_file(struct fat_fs_struct* fs,
                      struct fat_dir_entry_struct* dir_entry,
                      struct fat_dir_struct* parent_new, const char* file_new);
uint8_t fat_create_dir(struct fat_dir_struct* parent, const char* dir,
                       struct fat_dir_entry_struct* dir_entry);
#define fat_delete_dir fat_delete_file
#define fat_move_dir fat_move_file

void fat_get_file_modification_date(
    const struct fat_dir_entry_struct* dir_entry, uint16_t* year,
    uint8_t* month, uint8_t* day);
void fat_get_file_modification_time(
    const struct fat_dir_entry_struct* dir_entry, uint8_t* hour, uint8_t* min,
    uint8_t* sec);

uint8_t fat_get_dir_entry_of_path(struct fat_fs_struct* fs, const char* path,
                                  struct fat_dir_entry_struct* dir_entry);

offset_t fat_get_fs_size(const struct fat_fs_struct* fs);
offset_t fat_get_fs_free(const struct fat_fs_struct* fs);

/**
 * @}
 */

uint32_t fat_read_header(struct fat_fs_struct* fs);
cluster_t fat_get_next_cluster(const struct fat_fs_struct* fs,
                               cluster_t cluster_num);
offset_t fat_cluster_offset(const struct fat_fs_struct* fs,
                            cluster_t cluster_num);
uint8_t fat_dir_entry_read_callback(uint8_t* buffer, offset_t offset, void* p);
#if FAT_LFN_SUPPORT
uint8_t fat_calc_83_checksum(const uint8_t* file_name_83);
#endif

uint8_t fat_get_fs_free_16_callback(uint8_t* buffer, offset_t offset, void* p);
#if FAT_FAT32_SUPPORT
uint8_t fat_get_fs_free_32_callback(uint8_t* buffer, offset_t offset, void* p);
#endif

#if FAT_WRITE_SUPPORT
cluster_t fat_append_clusters(struct fat_fs_struct* fs, cluster_t cluster_num,
                              cluster_t count);
uint8_t fat_free_clusters(struct fat_fs_struct* fs, cluster_t cluster_num);
uint8_t fat_terminate_clusters(struct fat_fs_struct* fs, cluster_t cluster_num);
uint8_t fat_clear_cluster(const struct fat_fs_struct* fs,
                          cluster_t cluster_num);
uintptr_t fat_clear_cluster_callback(uint8_t* buffer, offset_t offset, void* p);
offset_t fat_find_offset_for_dir_entry(
    struct fat_fs_struct* fs, const struct fat_dir_struct* parent,
    const struct fat_dir_entry_struct* dir_entry);
uint8_t fat_write_dir_entry(const struct fat_fs_struct* fs,
                            struct fat_dir_entry_struct* dir_entry);
#if FAT_DATETIME_SUPPORT
void fat_set_file_modification_date(struct fat_dir_entry_struct* dir_entry,
                                    uint16_t year, uint8_t month, uint8_t day);
void fat_set_file_modification_time(struct fat_dir_entry_struct* dir_entry,
                                    uint8_t hour, uint8_t min, uint8_t sec);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
