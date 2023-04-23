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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <common/errno.h>
#include <ext/superblock.h>
#include <ext/blockgroup.h>
#include <ext/inode.h>
#include <ext/indirection.h>
#include <ext/bfsext_export.h>

/**
 * @brief Helper to get local inode index
 *
 * @param fs
 * @param inode
 * @param result
 * @return int
 */
BFSEXT_NO_EXPORT int ext_inode_get_local_inode(
  ext_fs_t* fs,
  uint64_t inode,
  uint64_t* result
) {
  // validate parameter
  if ( ! fs || ! result ) {
    return EINVAL;
  }
  // calculate block group and push it to result
  *result = ( inode - 1 ) % fs->superblock.s_inodes_per_group;
  // return success
  return EOK;
}

/**
 * @brief Method to read an inode
 *
 * @param fs
 * @param number
 * @param inode
 * @return int
 */
BFSEXT_NO_EXPORT int ext_inode_read_inode(
  ext_fs_t* fs,
  uint64_t number,
  ext_structure_inode_t* inode
) {
  // validate
  if ( ! fs || ! inode ) {
    return EINVAL;
  }
  // get inode size
  uint64_t inode_size;
  int result = ext_superblock_inode_size( fs, &inode_size );
  if ( EOK != result ) {
    return result;
  }
  // get block size
  uint64_t block_size;
  result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // get internal inode
  uint64_t number_local;
  result = ext_inode_get_local_inode( fs, number, &number_local );
  if ( EOK != result ) {
    return result;
  }
  // read block group descriptor
  ext_structure_block_group_descriptor_t descriptor;
  result = ext_blockgroup_read( fs, number_local, &descriptor );
  if ( EOK != result ) {
    return result;
  }
  // calculate offset
  uint64_t inode_offset = inode_size * ( number_local % fs->superblock.s_inodes_per_group );
  // allocate buffer
  uint8_t* buffer = malloc( block_size );
  if ( ! buffer ) {
    return ENOMEM;
  }
  // read data
  result = common_blockdev_bytes_read(
    fs->bdev,
    descriptor.bg_inode_table * block_size,
    buffer,
    block_size
  );
  if ( EOK != result ) {
    free( buffer );
    return ENOMEM;
  }
  // copy over data
  memcpy( inode, buffer + inode_offset, sizeof( *inode ) );
  // free data
  free( buffer );
  return EOK;
}
