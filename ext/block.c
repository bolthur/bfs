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
#include <string.h>
#include <stdlib.h>
#include <common/errno.h> // IWYU pragma: keep
#include <common/blockdev.h>
#include <ext/superblock.h>
#include <ext/block.h>
#include <ext/blockgroup.h>
#include <ext/fs.h>
#include <ext/structure.h>
#include <ext/bfsext_export.h>

/**
 * @brief Reads block bitmap
 *
 * @param fs
 * @param descriptor
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_block_read_bitmap(
  ext_fs_t* fs,
  ext_structure_block_group_descriptor_t* descriptor,
  uint32_t* value
) {
  // validate
  if ( ! fs || ! descriptor || ! value ) {
    return EINVAL;
  }
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // load block group
  result = common_blockdev_bytes_read(
    fs->bdev,
    descriptor->bg_block_bitmap * block_size,
    value,
    block_size
  );
  if ( EOK != result ) {
    return result;
  }
  // return success
  return EOK;
}

/**
 * @brief Write back block bitmap
 *
 * @param fs
 * @param descriptor
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_block_write_bitmap(
  ext_fs_t* fs,
  ext_structure_block_group_descriptor_t* descriptor,
  uint32_t* value
) {
  // validate
  if ( ! fs || ! descriptor || ! value ) {
    return EINVAL;
  }
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // load block group
  result = common_blockdev_bytes_write(
    fs->bdev,
    descriptor->bg_block_bitmap * block_size,
    value,
    block_size
  );
  if ( EOK != result ) {
    return result;
  }
  // return success
  return EOK;
}

/**
 * @brief Allocate a new block
 *
 * @param fs
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_block_allocate( ext_fs_t* fs, uint64_t* value ) {
  // validate
  if ( ! fs || ! value ) {
    return EINVAL;
  }
  // get block group count
  uint64_t bgc;
  int result = ext_superblock_blockgroup_count( fs, &bgc );
  if ( EOK != result ) {
    return result;
  }
  int64_t blockgroup = -1;
  for ( uint64_t i = 0; i < bgc && -1 == blockgroup; i++ ) {
    // read block group
    ext_structure_block_group_descriptor_t bgd;
    result = ext_blockgroup_read( fs, i, &bgd );
    if ( EOK != result ) {
      return result;
    }
    // check if has free blocks
    if ( bgd.bg_free_blocks_count ) {
      blockgroup = ( int64_t )i;
    }
  }
  // check for nothing found
  if ( -1 == blockgroup ) {
    return ENOSPC;
  }
  // read block group
  ext_structure_block_group_descriptor_t bgd;
  result = ext_blockgroup_read( fs, ( uint64_t )blockgroup, &bgd );
  if ( EOK != result ) {
    return result;
  }
  // get block size
  uint64_t block_size;
  result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // allocate space for bitmap
  uint8_t* bitmap = malloc( ( size_t )block_size );
  if ( ! bitmap ) {
    return ENOMEM;
  }
  // read bitmap
  memset( bitmap, 0, ( size_t )block_size );
  result = ext_block_read_bitmap( fs, &bgd, ( uint32_t* )bitmap );
  if ( EOK != result ) {
    free( bitmap );
    return result;
  }
  // search for free bit
  for ( uint64_t i = 0; i < fs->superblock.s_blocks_per_group / 32; i++ ) {
    // skip if bitmap is full
    if ( ( ( uint32_t* )bitmap )[ i ] == ~0U ) {
      continue;
    }
    for ( uint64_t j = 0; j < 32; j++ ) {
      if ( 0 == ( ( ( uint32_t* )bitmap )[ i ] & ( 1U << j ) ) ) {
        // mark as used in bitmap
        ( ( uint32_t* )bitmap )[ i ] |= ( 1U << j );
        // write back bitmap
        result = ext_block_write_bitmap( fs, &bgd, ( uint32_t* )bitmap );
        if ( EOK != result ) {
          free( bitmap );
          return result;
        }
        // update suberblock
        fs->superblock.s_free_blocks_count--;
        result = ext_superblock_write( fs, &fs->superblock );
        if ( EOK != result ) {
          free( bitmap );
          return result;
        }
        // update block group
        bgd.bg_free_blocks_count--;
        result = ext_blockgroup_write( fs, ( uint64_t )blockgroup, &bgd );
        if ( EOK != result ) {
          free( bitmap );
          return result;
        }
        uint64_t block_number = fs->superblock.s_blocks_per_group * ( uint64_t )blockgroup
          + ( i * 32 + j ) + fs->superblock.s_first_data_block;
        // allocate space and fill new block with 0
        uint8_t* new_block = malloc( ( size_t )block_size );
        if ( ! new_block ) {
          free( bitmap );
          return ENOMEM;
        }
        memset( new_block, 0, ( size_t )block_size );
        result = common_blockdev_bytes_write( fs->bdev, block_number * block_size, new_block, block_size );
        if ( EOK != result ) {
          free( bitmap );
          return result;
        }
        free( bitmap );
        free( new_block );
        // return success
        *value = block_number;
        return EOK;
      }
    }
  }
  free( bitmap );
  // return no space left
  return ENOSPC;
}

/**
 * @brief Deallocate a block
 *
 * @param fs
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_block_deallocate( ext_fs_t* fs, uint64_t value ) {
  if ( ! fs || ! value ) {
    return EINVAL;
  }

  value -= fs->superblock.s_first_data_block;
  uint64_t bgnum = value / fs->superblock.s_blocks_per_group;
  value %= fs->superblock.s_blocks_per_group;

  ext_structure_block_group_descriptor_t bgd;
  int result = ext_blockgroup_read( fs, bgnum, &bgd );
  if ( EOK != result ) {
    return result;
  }
  // get block size
  uint64_t block_size;
  result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // allocate space for bitmap
  uint8_t* bitmap = malloc( ( size_t )block_size );
  if ( ! bitmap ) {
    return ENOMEM;
  }
  // read bitmap
  memset( bitmap, 0, ( size_t )block_size );
  result = ext_block_read_bitmap( fs, &bgd, ( uint32_t* )bitmap );
  if ( EOK != result ) {
    free( bitmap );
    return result;
  }
  // free in bitmap
  ( ( uint32_t* )bitmap )[ value / 32 ] &= ~( 1U << ( value % 32 ) );
  // write back bitmap
  result = ext_block_write_bitmap( fs, &bgd, ( uint32_t* )bitmap );
  if ( EOK != result ) {
    free( bitmap );
    return result;
  }
  // free bitmap again
  free( bitmap );
  // update superblock
  fs->superblock.s_free_blocks_count++;
  result = ext_superblock_write( fs, &fs->superblock );
  if ( EOK != result ) {
    return result;
  }
  // update block group descriptor
  bgd.bg_free_blocks_count++;
  result = ext_blockgroup_write( fs, bgnum, &bgd );
  if ( EOK != result ) {
    return result;
  }
  // return success
  return EOK;
}
