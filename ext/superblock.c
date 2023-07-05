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
#include <endian.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <common/blockdev.h>
#include <common/errno.h>
#include <ext/blockgroup.h>
#include <ext/superblock.h>
#include <ext/fs.h>
#include <ext/structure.h>
#include <ext/bfsext_export.h>

/**
 * @brief Read superblock
 *
 * @param bdev
 * @param superblock
 * @return int
 */
BFSEXT_NO_EXPORT int ext_superblock_read(
  ext_fs_t* fs,
  ext_structure_superblock_t* superblock
) {
  return common_blockdev_bytes_read(
    fs->bdev,
    1024,
    superblock,
    sizeof( ext_structure_superblock_t )
  );
}

/**
 * @brief Write back superblock
 *
 * @param fs
 * @param superblock
 * @return int
 */
BFSEXT_NO_EXPORT int ext_superblock_write(
  ext_fs_t* fs,
  ext_structure_superblock_t* superblock
) {
  if ( ! fs || ! superblock ) {
    return EINVAL;
  }
  // get group count
  uint64_t group_count;
  int result = ext_superblock_blockgroup_count( fs, &group_count );
  if ( EOK != result ) {
    return result;
  }
  // get block size
  uint64_t block_size;
  result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // allocate buffer
  uint8_t* buffer = malloc( ( size_t )block_size );
  if ( ! buffer ) {
    return ENOMEM;
  }
  // loop through groups
  for ( uint64_t idx = 0; idx < group_count; idx++ ) {
    // check for block group
    bool has_superblock;
    result = ext_blockgroup_has_superblock( fs, idx, &has_superblock );
    if ( EOK != result ) {
      free( buffer );
      return result;
    }
    // skip if not there
    if ( ! has_superblock ) {
      continue;
    }
    // determine start
    uint64_t start = 0 == idx
      ? 1024
      : ( fs->superblock.s_first_data_block + idx * fs->superblock.s_blocks_per_group ) * block_size;
    // read block
    result = common_blockdev_bytes_read( fs->bdev, start, buffer, block_size );
    if ( EOK != result ) {
      free( buffer );
      return result;
    }
    // update superblock
    if ( 0 == idx && 1024 < block_size ) {
      memcpy( buffer + 1024, superblock, sizeof( *superblock ) );
    } else {
      memcpy( buffer, superblock, sizeof( *superblock ) );
    }
    // write back
    result = common_blockdev_bytes_write( fs->bdev, start, buffer, block_size );
    if ( EOK != result ) {
      free( buffer );
      return result;
    }
  }
  // free buffer
  free( buffer );
  return EOK;
}

/**
 * @brief Validate
 *
 * @param fs
 * @return int
 */
BFSEXT_NO_EXPORT int ext_superblock_check( ext_fs_t* fs ) {
  if ( ! fs ) {
    return EINVAL;
  }
  // check magic
  if ( fs->superblock.s_magic != EXT_SUPERBLOCK_MAGIC ) {
    return ENOTSUP;
  }
  // validate inode count
  if ( 0 == fs->superblock.s_inodes_count ) {
    return ENOTSUP;
  }
  // validate block count
  if ( 0 == fs->superblock.s_blocks_count ) {
    return ENOTSUP;
  }
  // validate block count
  if ( 0 == fs->superblock.s_blocks_count ) {
    return ENOTSUP;
  }
  // validate blocks per group
  if ( 0 == fs->superblock.s_blocks_per_group ) {
    return ENOTSUP;
  }
  // validate inodes per group
  if ( 0 == fs->superblock.s_inodes_per_group ) {
    return ENOTSUP;
  }
  // validate inode size for old revision
  if (
    EXT_GOOD_OLD_REV == fs->superblock.s_rev_level
    && 128 > fs->superblock.s_inode_size
  ) {
    return ENOTSUP;
  }
  // get block size
  uint64_t bsize;
  int result = ext_superblock_block_size( fs, &bsize );
  if ( EOK != result ) {
    return result;
  }
  // validate inode size for dynamic revision
  uint64_t inode_size = fs->superblock.s_inode_size;
  if (
    EXT_DYNAMIC_REV == fs->superblock.s_rev_level
    && (
      ! ( 0 == ( inode_size & ( inode_size - 1 ) ) )
      || ( inode_size > bsize )
    )
  ) {
    return ENOTSUP;
  }
  // validate first inode block
  if (
    EXT_GOOD_OLD_REV == fs->superblock.s_rev_level
    && 11 > fs->superblock.s_first_ino
  ) {
    return ENOTSUP;
  }
  // return success
  return EOK;
}

/**
 * @brief Function to get block size
 *
 * @param fs
 * @param block_size
 * @return int
 */
BFSEXT_NO_EXPORT int ext_superblock_block_size( ext_fs_t* fs, uint64_t* block_size ) {
  if ( ! fs || ! block_size ) {
    return EINVAL;
  }
  // set block size
  *block_size = 1024 << fs->superblock.s_log_block_size;
  // return success
  return EOK;
}

/**
 * @brief Helper to check if is power of
 *
 * @param a
 * @param b
 * @return int
 */
BFSEXT_NO_EXPORT int superblock_is_power_of( uint64_t a, uint64_t b )
{
  while (1) {
    if (a < b)
      return 0;
    if (a == b)
      return 1;
    if ((a % b) != 0)
      return 0;
    a = a / b;
  }
}

/**
 * @brief Get fragment size
 *
 * @param fs
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_superblock_frag_size( ext_fs_t* fs, uint64_t* value ) {
  if ( ! fs || ! value ) {
    return EINVAL;
  }
  *value = 1024 << fs->superblock.s_log_frag_size;
  return EOK;
}

/**
 * @brief Get total group by blocks
 *
 * @param fs
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_superblock_total_group_by_blocks( ext_fs_t* fs, uint64_t* value ) {
  if ( ! fs || ! value ) {
    return EINVAL;
  }
  *value = fs->superblock.s_blocks_count / fs->superblock.s_blocks_per_group +
    ( fs->superblock.s_blocks_count % fs->superblock.s_blocks_per_group ? 1 : 0 );
  return EOK;
}

/**
 * @brief Get total group by inode
 *
 * @param fs
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_superblock_total_group_by_inode( ext_fs_t* fs, uint64_t* value ) {
  if ( ! fs || ! value ) {
    return EINVAL;
  }
  *value = fs->superblock.s_inodes_count / fs->superblock.s_inodes_per_group +
    ( fs->superblock.s_inodes_count % fs->superblock.s_inodes_per_group ? 1 : 0 );
  return EOK;
}

/**
 * @brief Get start
 *
 * @param fs
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_superblock_start( ext_fs_t* fs, uint64_t* value ) {
  if ( ! fs || ! value ) {
    return EINVAL;
  }
  // fetch block size
  uint64_t size;
  int result = ext_superblock_block_size( fs, &size );
  if ( EOK != result ) {
    return result;
  }
  // populate result
  *value = 1024 < size ? 0 : 1;
  return EOK;
}

/**
 * @brief Get inode size
 *
 * @param fs
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_superblock_inode_size( ext_fs_t* fs, uint64_t* value ) {
  if ( ! fs || ! value ) {
    return EINVAL;
  }
  *value = 0 == fs->superblock.s_rev_level ? 128 : fs->superblock.s_inode_size;
  return EOK;
}

/**
 * @brief Get blockgroup count
 *
 * @param fs
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_superblock_blockgroup_count( ext_fs_t* fs, uint64_t* value ) {
  if ( ! fs || ! value ) {
    return EINVAL;
  }
  ext_structure_superblock_t* sb = &fs->superblock;
  *value = ( sb->s_blocks_count - sb->s_first_data_block + ( sb->s_blocks_per_group - 1 ) ) /
    sb->s_blocks_per_group;
  return EOK;
}
