// Copyright (C) 2022 - 2023 bolthur project.
//
// This file is part of bolthur/bfs.
//
// bolthur/bfs is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// bolthur/bfs is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with bolthur/bfs.  If not, see <http://www.gnu.org/licenses/>.

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
 * @param lba
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
  uint64_t fat_bit_offset = 0;
  // determine fat sector and index
  if ( FAT_FAT12 == fs->type ) {
    fat_bit_offset = current * 12;
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
    if ( 0 == ( fat_bit_offset & 7 ) ) {
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

/**
 * @brief Method to get a free cluster
 *
 * @param fs
 * @param cluster
 * @return int
 */
BFSFAT_NO_EXPORT int fat_cluster_get_free( fat_fs_t* fs, uint64_t* cluster ) {
  uint64_t index;
  // loop through data sectors and look for a free one
  for ( index = 0; index < fs->data_sectors; index++ ) {
    uint64_t internal_cluster;
    // get next cluster
    int result = fat_cluster_next( fs, index, &internal_cluster );
    if ( EOK != result ) {
      return result;
    }
    // check if it's free
    if ( FAT_CLUSTER_UNUSED == internal_cluster ) {
      break;
    }
  }
  // handle no free found
  if ( index >= fs->data_sectors ) {
    return ENOSPC;
  }
  // update cluster
  *cluster = index;
  // return success
  return EOK;
}

/**
 * @brief Set a fat cluster to a value
 *
 * @param fs
 * @param cluster
 * @param value
 * @return int
 */
BFSFAT_EXPORT int fat_cluster_set_cluster(
  fat_fs_t* fs,
  uint64_t cluster,
  uint64_t value
) {
  // allocate buffer
  uint8_t* buffer = malloc( fs->superblock.bytes_per_sector );
  if ( ! buffer ) {
    return ENOMEM;
  }
  // bunch of variables
  uint64_t fat_sector;
  uint64_t fat_index;
  uint64_t fat_bit_offset = 0;
  // determine fat sector and index
  if ( FAT_FAT12 == fs->type ) {
    fat_bit_offset = cluster * 12;
    fat_sector = fat_bit_offset / SECTOR_BITS
      + fs->superblock.reserved_sector_count;
    fat_bit_offset %= SECTOR_BITS;
    fat_index = fat_bit_offset / 8;
  } else if ( FAT_FAT16 == fs->type ) {
    uint64_t cluster_size = fs->superblock.bytes_per_sector
      * fs->superblock.sectors_per_cluster;
    fat_sector = ( cluster * sizeof( uint16_t ) ) / cluster_size
      + fs->first_fat_sector;
    fat_index = ( cluster * sizeof( uint16_t ) ) % cluster_size;
  } else if ( FAT_FAT32 == fs->type ) {
    uint64_t cluster_size = fs->superblock.bytes_per_sector
      * fs->superblock.sectors_per_cluster;
    fat_sector = ( cluster * sizeof( uint32_t ) ) / cluster_size
      + fs->first_fat_sector;
    fat_index = ( cluster * sizeof( uint32_t ) ) % cluster_size;
  } else {
    free( buffer );
    return ENOTSUP;
  }
  // read sector to memory
  int result = common_blockdev_bytes_read(
    fs->bdev,
    fat_sector * fs->bdev->bdif->block_size,
    buffer,
    fs->bdev->bdif->block_size
  );
  if ( EOK != result ) {
    free( buffer );
    return result;
  }
  if ( FAT_FAT12 == fs->type ) {
    // read fat entry
    uint16_t* pvalue = ( uint16_t* )&buffer[ fat_index ];
    uint16_t rvalue = ( uint16_t )value & 0xFFF;
    // adjust value if necessary
    if ( 0 == ( fat_bit_offset & 7 ) ) {
      *pvalue &= ( uint16_t )~0x0FFF;
      *pvalue |= rvalue;
    } else {
      *pvalue &= ( uint16_t )~0xFFF0;
      *pvalue |= ( rvalue << 4 );
    }
  } else if ( FAT_FAT16 == fs->type ) {
    // read fat entry
    uint16_t* pvalue = ( uint16_t* )&buffer[ fat_index ];
    *pvalue = ( uint16_t )value;
  } else if ( FAT_FAT32 == fs->type ) {
    // read fat entry
    uint32_t* pvalue = ( uint32_t* )&buffer[ fat_index ];
    *pvalue = ( uint32_t )value;
  } else {
    free( buffer );
    return ENOTSUP;
  }
  // write sector to memory
  result = common_blockdev_bytes_write(
    fs->bdev,
    fat_sector * fs->bdev->bdif->block_size,
    buffer,
    fs->bdev->bdif->block_size
  );
  // handle error
  if ( EOK != result ) {
    free( buffer );
    return result;
  }
  // return success
  free( buffer );
  return EOK;
}

/**
 * @brief Helper to get cluster by number in cluster chain
 *
 * @param fs
 * @param cluster
 * @param num
 * @param target
 * @return int
 */
int fat_cluster_get_by_num(
  fat_fs_t* fs,
  uint64_t cluster,
  uint64_t num,
  uint64_t* target
) {
  // validate parameter
  if ( ! fs || ! target || ! cluster ) {
    return EINVAL;
  }
  // handle num is 0 => just return cluster
  if ( ! num ) {
    *target = cluster;
    return EOK;
  }
  uint64_t end_value;
  int result = fat_cluster_get_chain_end_value( fs, &end_value );
  if ( EOK != result ) {
    return result;
  }
  // walk the chain until index
  uint64_t current = 0;
  for ( uint64_t index = 0; index < num; index++ ) {
    // handle begin
    if ( 0 == current ) {
      current = cluster;
      continue;
    }
    // variable for next
    uint64_t next;
    // fetch next cluster
    result = fat_cluster_next( fs, current, &next );
    // handle error
    if ( EOK != result ) {
      return result;
    }
    // handle chain end reached
    if ( next >= end_value ) {
      return ENXIO;
    }
    // overwrite current with next
    current = next;
  }
  // set target
  *target = current;
  return EOK;
}

/**
 * @brief Method to get cluster chain end value
 *
 * @param fs
 * @param end
 * @return int
 */
BFSFAT_NO_EXPORT int fat_cluster_get_chain_end_value(
  fat_fs_t* fs,
  uint64_t* end
) {
  if ( ! fs || ! end ) {
    return EINVAL;
  }
  if ( FAT_FAT12 == fs->type ) {
    *end = FAT_FAT12_CLUSTER_CHAIN_END;
  } else if ( FAT_FAT16 == fs->type ) {
    *end = FAT_FAT16_CLUSTER_CHAIN_END;
  } else if ( FAT_FAT32 == fs->type ) {
    *end = FAT_FAT32_CLUSTER_CHAIN_END;
  } else {
    return EINVAL;
  }
  return EOK;
}
