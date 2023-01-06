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

// IWYU pragma: no_include <errno.h>
#include <stdlib.h>
#include <common/blockdev.h>
#include <common/errno.h>
#include <fat/cluster.h>
#include <fat/structure.h>
#include <fat/fs.h>
#include <fat/bfsfat_export.h>

#define SECTOR_BITS (512 * 8)

/**
 * @brief Transform cluster to logical block address
 *
 * @param fs
 * @param cluster
 * @return int
 */
BFSFAT_NO_EXPORT int fat_cluster_to_lba(
  fat_fs_t* fs,
  uint64_t cluster,
  uint64_t* lba
) {
  // validate parameter
  if ( ! fs || ! lba ) {
    return EINVAL;
  }
  // set lba
  *lba = ( ( cluster - 2 ) * fs->superblock.sectors_per_cluster )
    + fs->first_data_sector;
  // return success
  return EOK;
}

/**
 * @brief Get next cluster
 *
 * @param fs
 * @param current
 * @param next
 * @return int
 */
BFSFAT_NO_EXPORT int fat_cluster_next(
  fat_fs_t* fs,
  uint64_t current,
  uint64_t* next
) {
  // allocate buffer
  uint8_t* buffer = malloc( fs->superblock.bytes_per_sector );
  if ( ! buffer ) {
    return ENOMEM;
  }
  // bunch of variables
  uint64_t fat_sector;
  uint64_t fat_index;
  // determine fat sector and index
  if ( FAT_FAT12 == fs->type ) {
    uint64_t fat_bit_offset = current * 12;
    fat_sector = fat_bit_offset / SECTOR_BITS
      + fs->superblock.reserved_sector_count;
    fat_bit_offset %= SECTOR_BITS;
    fat_index = fat_bit_offset / 8;
  } else if ( FAT_FAT16 == fs->type ) {
    uint64_t cluster_size = fs->superblock.bytes_per_sector
      * fs->superblock.sectors_per_cluster;
    fat_sector = ( current * sizeof( uint16_t ) ) / cluster_size
      + fs->first_fat_sector;
    fat_index = ( current * sizeof( uint16_t ) ) % cluster_size;
  } else if ( FAT_FAT32 == fs->type ) {
    uint64_t cluster_size = fs->superblock.bytes_per_sector
      * fs->superblock.sectors_per_cluster;
    fat_sector = ( current * sizeof( uint32_t ) ) / cluster_size
      + fs->first_fat_sector;
    fat_index = ( current * sizeof( uint32_t ) ) % cluster_size;
  } else {
    free( buffer );
    return ENOTSUP;
  }
  // read sector to memory
  int result = common_blockdev_bytes_read(
    fs->bdev,
    fat_sector * fs->bdev->bdif->block_size,
    buffer,
    fs->superblock.bytes_per_sector
  );
  if ( EOK != result ) {
    free( buffer );
    return result;
  }
  if ( FAT_FAT12 == fs->type ) {
    // read fat entry
    uint16_t value = *( uint16_t* )&buffer[ fat_index ];
    // adjust value if necessary
    if ( 0 == ( current & 7 ) ) {
      value = value & 0x0FFF;
    } else {
      value >>= 4;
    }
    // push to target
    if ( 0xff5 < value ) {
      *next = ( uint64_t )value | 0xfffff000;
    } else {
      *next = value;
    }
  } else if ( FAT_FAT16 == fs->type ) {
    // read fat entry
    uint16_t value = *( uint16_t* )&buffer[ fat_index ];
    // push to target
    if ( 0xfff5 < value ) {
      *next = ( uint64_t )value | 0xffff0000;
    } else {
      *next = value;
    }
  } else if ( FAT_FAT32 == fs->type ) {
    // read fat entry
    uint32_t value = *( uint32_t* )&buffer[ fat_index ];
    // cap value
    if ( 0xffffff5 < value ) {
      value |= 0xf0000000;
    }
    // push to target
    *next = value;
  } else {
    free( buffer );
    return ENOTSUP;
  }
  // return success
  free( buffer );
  return EOK;
}
