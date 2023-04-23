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

#include <string.h>
#include <stdlib.h>
#include <common/errno.h>
#include <ext/blockgroup.h>
#include <ext/superblock.h>
#include <ext/bfsext_export.h>

/**
 * @brief Helper to check whether block group contains a superblock
 *
 * @param superblock
 * @param blockgroup
 * @param result
 * @return int
 */
BFSEXT_NO_EXPORT int ext_blockgroup_has_superblock(
  ext_fs_t* fs,
  uint64_t blockgroup,
  bool* result
) {
  // validate parameter
  if ( ! fs || ! result ) {
    return EINVAL;
  }
  // check for sparse super
  if ( ! ( fs->superblock.s_feature_ro_compat & EXT_SUPERBLOCK_EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER ) ) {
    *result = true;
    return EOK;
  }
  if ( 1 >= blockgroup ) {
    *result = true;
    return EOK;
  }
  if ( ! ( blockgroup & 1 ) ) {
    *result = false;
    return EOK;
  }
  *result = superblock_is_power_of( blockgroup, 7 )
    || superblock_is_power_of( blockgroup, 5 )
    || superblock_is_power_of( blockgroup, 3 );
  return EOK;
}

/**
 * @brief Helper to get block group by inode
 *
 * @param superblock
 * @param inode
 * @param result
 * @return int
 */
BFSEXT_NO_EXPORT int ext_blockgroup_get_by_inode(
  ext_fs_t* fs,
  uint64_t inode,
  uint64_t* result
) {
  // validate parameter
  if ( ! fs || ! result ) {
    return EINVAL;
  }
  // calculate block group and push it to result
  *result = ( inode - 1 ) / fs->superblock.s_inodes_per_group;
  // return success
  return EOK;
}

/**
 * @brief Read block group data
 *
 * @param fs
 * @param group
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_blockgroup_read(
  ext_fs_t* fs,
  uint64_t group,
  ext_structure_block_group_descriptor_t* value
) {
  if ( ! fs || ! value ) {
    return EINVAL;
  }
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // calculate block number
  uint64_t block_number;
  result = ext_superblock_start( fs, &block_number );
  if ( EOK != result ) {
    return result;
  }
  block_number += 1
    + ( group * sizeof( ext_structure_block_group_descriptor_t ) / block_size);
  uint8_t* tmp = malloc( block_size );
  if ( ! tmp ) {
    return ENOMEM;
  }
  // read block group
  result = common_blockdev_bytes_read(
    fs->bdev,
    block_number * block_size,
    tmp,
    block_size
  );
  if ( EOK != result ) {
    free( tmp );
    return result;
  }
  // copy over content
  memcpy(
    value,
    tmp + ( ( group * sizeof( ext_structure_block_group_descriptor_t ) ) % block_size ),
    sizeof( ext_structure_block_group_descriptor_t )
  );
  free( tmp );
  return EOK;
}

/**
 * @brief Write block group descriptor
 *
 * @param fs
 * @param group
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_blockgroup_write(
  ext_fs_t* fs,
  uint64_t group,
  ext_structure_block_group_descriptor_t* value
) {
  if ( ! fs || ! value ) {
    return EINVAL;
  }
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // calculate block number
  uint64_t block_number;
  result = ext_superblock_start( fs, &block_number );
  if ( EOK != result ) {
    return result;
  }
  block_number += 1
    + ( group * sizeof( ext_structure_block_group_descriptor_t ) / block_size);
  uint8_t* tmp = malloc( block_size );
  if ( ! tmp ) {
    return ENOMEM;
  }
  // read block group
  result = common_blockdev_bytes_read(
    fs->bdev,
    block_number * block_size,
    tmp,
    block_size
  );
  if ( EOK != result ) {
    free( tmp );
    return result;
  }
  // copy over content
  memcpy(
    tmp + ( ( group * sizeof( ext_structure_block_group_descriptor_t ) ) % block_size ),
    value,
    sizeof( ext_structure_block_group_descriptor_t )
  );
  // read block group
  result = common_blockdev_bytes_write(
    fs->bdev,
    block_number * block_size,
    tmp,
    block_size
  );
  if ( EOK != result ) {
    free( tmp );
    return result;
  }
  free( tmp );
  return EOK;
}
