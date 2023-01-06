/**
 * Copyright (C) 2022 bolthur project.
 *
 * This file is part of bolthur/bfs.
 *
 * bolthur/bfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bolthur/bfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bolthur/bfs.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#ifndef _FAT_STRUCTURE_H
#define _FAT_STRUCTURE_H

#ifdef __cplusplus
extern "C" {
#endif

// entry specifiers
#define FAT_DIRECTORY_ENTRY_AVAILABLE 0x00
#define FAT_DIRECTORY_ENTRY_PENDING_DELETION 0x05
#define FAT_DIRECTORY_ENTRY_DOT_ENTRY 0x2E
#define FAT_DIRECTORY_ENTRY_ERASED_AVAILABLE 0xE5
// file attribute defines
#define FAT_DIRECTORY_FILE_ATTRIBUTE_READ_ONLY 0x01
#define FAT_DIRECTORY_FILE_ATTRIBUTE_HIDDEN 0x02
#define FAT_DIRECTORY_FILE_ATTRIBUTE_SYSTEM 0x04
#define FAT_DIRECTORY_FILE_ATTRIBUTE_VOLUME_ID 0x08
#define FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY 0x10
#define FAT_DIRECTORY_FILE_ATTRIBUTE_ARCHIVE 0x20
#define FAT_DIRECTORY_FILE_ATTRIBUTE_LONG_FILE_NAME \
  ( FAT_DIRECTORY_FILE_ATTRIBUTE_READ_ONLY | FAT_DIRECTORY_FILE_ATTRIBUTE_HIDDEN \
  | FAT_DIRECTORY_FILE_ATTRIBUTE_SYSTEM | FAT_DIRECTORY_FILE_ATTRIBUTE_VOLUME_ID )
// helper to extract specific information from time field
#define FAT_DIRECTORY_TIME_EXTRACT_HOUR(x) ((x & 0x7C00) >> 11)
#define FAT_DIRECTORY_TIME_EXTRACT_MINUTE(x) ((x & 0x3F0) >> 5)
#define FAT_DIRECTORY_TIME_EXTRACT_SECOND(x) (x & 0x1F)
// helper to extract specific information from date field
#define FAT_DIRECTORY_DATE_EXTRACT_HOUR(x) ((x & 0xFE00) >> 9)
#define FAT_DIRECTORY_DATE_EXTRACT_MINUTE(x) ((x & 0x1E0) >> 5)
#define FAT_DIRECTORY_DATE_EXTRACT_SECOND(x) (x & 0x1F)

#pragma pack(push, 1)

/**
 * @brief Fat12 / Fat16 related data of superblock aka bpb
 */
typedef struct {
  /**
   * @brief drive number
   */
  uint8_t bios_drive_number;
  /**
   * @brief Reserved
   */
  uint8_t reserved;
  /**
   * @brief Boot signature, must be 0x28 or 0x29
   */
  uint8_t boot_signature;
  /**
   * @brief Serial number of the volume
   */
  uint32_t volume_id;
  /**
   * @brief Volume label
   */
  char volume_label[ 11 ];
  /**
   * @brief System identifier string, either FAT12 or FAT16
   */
  char fat_type_label[ 8 ];
  /**
   * @brief Remaining space for boot code
   */
  uint8_t boot_code[ 448 ];
} fat_structure_superblock_fat_t;

/**
 * @brief Fat32 related data of superblock aka bpb
 */
typedef struct {
  /**
   * @brief Sectors per fat
   */
  uint32_t table_size_32;
  /**
   * @brief Some flags
   */
  uint16_t extended_flags;
  /**
   * @brief Fat version number
   */
  uint16_t fat_version;
  /**
   * @brief Cluster number of the root directory
   */
  uint32_t root_cluster;
  /**
   * @brief Fat32 file system info structure
   * @see fat_structure_fat32_fsinfo_t
   */
  uint16_t fat_info;
  /**
   * @brief Sector number of backup boot sector
   */
  uint16_t backup_boot_sector;
  /**
   * @brief Reserved space
   */
  uint8_t reserved0[ 12 ];
  /**
   * @brief Drive number
   */
  uint8_t drive_number;
  /**
   * @brief More reserved space
   */
  uint8_t reserved1;
  /**
   * @brief Signature, must be 0x28 or 0x29
   */
  uint8_t boot_signature;
  /**
   * @brief Serial number of the volume
   */
  uint32_t volume_id;
  /**
   * @brief Volume label
   */
  char volume_label[ 11 ];
  /**
   * @brief System identifier string, either FAT12 or FAT16
   */
  char fat_type_label[ 8 ];
  /**
   * @brief Remaining space for boot code
   */
  uint8_t boot_code[ 420 ];
} fat_structure_superblock_fat32_t;

/**
 * @brief Fat superblock aka bpb
 */
typedef struct {
  /**
   * @brief Instruction to jump to boot code
   */
  uint8_t bootjmp[ 3 ];
  /**
   * @brief OEM identifier
   */
  char oem_name[ 8 ];
  /**
   * @brief Amount of bytes per sector
   */
  uint16_t bytes_per_sector;
  /**
   * @brief Amount of sectors per cluster
   */
  uint8_t sectors_per_cluster;
  /**
   * @brief Amount of reserved sectors
   */
  uint16_t reserved_sector_count;
  /**
   * @brief Amount of file allocation tables
   */
  uint8_t table_count;
  /**
   * @brief Amount of root directory entries ( only FAT12 and FAT16 )
   */
  uint16_t root_entry_count;
  /**
   * @brief Total sector count in volume, 0 when more than 65535 sectors are within the volume
   */
  uint16_t total_sectors_16;
  /**
   * @brief Media descriptor type
   */
  uint8_t media_type;
  /**
   * @brief Number of sectors per fat ( only FAT12 and FAT16 )
   */
  uint16_t table_size_16;
  /**
   * @brief Number of sectors per track
   */
  uint16_t sectors_per_track;
  /**
   * @brief Number of heads / sides on storage media
   */
  uint16_t head_side_count;
  /**
   * @brief Number of hidden sectors
   */
  uint32_t hidden_sector_count;
  /**
   * @brief Large sector count, only set if more than 65535 are existing
   */
  uint32_t total_sectors_32;
  /**
   * @brief Extended boot record data
   */
  union {
    /**
     * @brief FAT12 / FAT16 related data
     * @see fat_structure_superblock_fat_t
     */
    fat_structure_superblock_fat_t fat;
    /**
     * @brief FAT32 related data
     * @see fat_structure_superblock_fat32_t
     */
    fat_structure_superblock_fat32_t fat32;
    /**
     * @brief Raw extension data
     */
    uint8_t raw[ 474 ];
  } extended;
  /**
   * @brief Bootable partition sector, must be 0xAA55
   */
  uint16_t boot_sector_signature;
} fat_structure_superblock_t;
static_assert(
  512 == sizeof( fat_structure_superblock_t ),
  "invalid fat_structure_superblock_t size!"
);

/**
 * @brief Exfat superblock aka bpb
 */
typedef struct {
  /**
   * @brief Instruction to jump to boot code
   */
  uint8_t bootjmp[ 3 ];
  /**
   * @brief OEM identifier
   */
  char oem_name[ 8 ];
  /**
   * @brief Data that should be zero
   */
  uint8_t sbz[ 53 ];
  /**
   * @brief Partition offset
   */
  uint64_t partition_offset;
  /**
   * @brief Volume length
   */
  uint64_t volume_length;
  /**
   * @brief Fat offset in sectors
   */
  uint32_t fat_offset;
  /**
   * @brief Fat size in sectors
   */
  uint32_t fat_length;
  /**
   * @brief cluster heap offset in sectors
   */
  uint32_t cluster_heap_offset;
  /**
   * @brief Cluster count
   */
  uint32_t cluster_count;
  /**
   * @brief Root directory start cluster
   */
  uint32_t root_dir_cluster;
  /**
   * @brief Serial number of partition
   */
  uint32_t serial_number;
  /**
   * @brief Filesystem revision
   */
  uint16_t revision;
  /**
   * @brief Some flags
   */
  uint16_t flags;
  /**
   * @brief Sector shift
   */
  uint8_t sector_shift;
  /**
   * @brief Cluster shift
   */
  uint8_t cluster_shift;
  /**
   * @brief Number of fats
   */
  uint8_t fat_count;
  /**
   * @brief Selected drive
   */
  uint8_t selected_drive;
  /**
   * @brief Percentage in use
   */
  uint8_t percentage_used;
  /**
   * @brief Reserved data
   */
  uint8_t reserved[ 7 ];
} fat_structure_superblock_exfat_t;
static_assert(
  120 == sizeof( fat_structure_superblock_exfat_t ),
  "invalid fat_structure_superblock_exfat_t size!"
);

/**
 * @brief Normal fat directory entry
 */
typedef struct {
  /**
   * @brief Entry name
   */
  char name[ 8 ];
  /**
   * @brief Entry extension
   */
  char extension[ 3 ];
  /**
   * @brief Entry attributes
   */
  uint8_t attributes;
  /**
   * @brief Reserved, should be ignored
   */
  uint8_t reserved0;
  /**
   * @brief Creation time in tenths of a second
   */
  uint8_t creation_time_tenths;
  /**
   * @brief Entry creation time
   */
  uint16_t creation_time;
  /**
   * @brief Entry creation date
   */
  uint16_t creation_date;
  /**
   * @brief Date of last access
   */
  uint16_t last_accessed_date;
  /**
   * @brief Upper 16 bit of first cluster ( only for FAT32, everything else is 0 )
   */
  uint16_t first_cluster_upper;
  /**
   * @brief Time of last modification
   */
  uint16_t last_modification_time;
  /**
   * @brief Date of last modification
   */
  uint16_t last_modification_date;
  /**
   * @brief Lower 16 bit of first cluster
   */
  uint16_t first_cluster_lower;
  /**
   * @brief Entry size, 0 for folders
   */
  uint32_t file_size;
} fat_structure_directory_entry_t;
static_assert(
  32 == sizeof( fat_structure_directory_entry_t ),
  "invalid fat_structure_directory_entry_t size!"
);

/**
 * @brief Long file name structure
 */
typedef struct {
  /**
   * @brief Order number of this sequence entry
   */
  uint8_t order;
  /**
   * @brief First five 2-byte characters
   */
  uint8_t first_five_two_byte[ 10 ];
  /**
   * @brief Attribute information, always 0x0F
   */
  uint8_t attribute;
  /**
   * @brief Long entry type
   */
  uint8_t type;
  /**
   * @brief Checksum
   */
  uint8_t checksum;
  /**
   * @brief Next six 2-byte characters
   */
  uint8_t next_six_two_byte[ 12 ];
  /**
   * @brief Always zero
   */
  uint16_t zero;
  /**
   * @brief final two 2-byte characters
   */
  uint8_t final_two_byte[ 4 ];
} fat_structure_directory_entry_long_t;
static_assert(
  32 == sizeof( fat_structure_directory_entry_long_t ),
  "invalid fat_structure_directory_entry_long_t size!"
);

/**
 * @brief Fat32 file system info object
 */
typedef struct {
  /**
   * @brief Lead signature, must be 0x41615252
   */
  uint32_t lead_signature;
  /**
   * @brief Reserved bytes that shouldn't be used
   */
  uint8_t reserved[ 480 ];
  /**
   * @brief Second signature, must be 0x61417272
   */
  uint32_t signature;
  /**
   * @brief Amount of known free clusters, unknown if value is 0xFFFFFFFF
   */
  uint32_t known_free_cluster_count;
  /**
   * @brief Available cluster start, unknown if value is 0xFFFFFFFF
   */
  uint32_t available_cluster_start;
  /**
   * @brief Reserved area
   */
  uint8_t reserved2[ 12 ];
  /**
   * @brief Trailing signature, must be 0xAA550000
   */
  uint32_t trail_signature;
} fat_structure_fat32_fsinfo_t;
static_assert(
  512 == sizeof( fat_structure_fat32_fsinfo_t ),
  "invalid fat_structure_fat32_fsinfo_t size!"
);

#pragma pack(pop)

typedef enum {
  FAT_FAT12 = 1,
  FAT_FAT16 = 2,
  FAT_FAT32 = 3,
} fat_type_t;

#ifdef __cplusplus
}
#endif

#endif
