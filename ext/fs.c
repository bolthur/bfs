// Copyright (C) 2022 - 2026 bolthur project.
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <common/errno.h> // IWYU pragma: keep
#include <common/blockdev.h>
#include <ext/fs.h>
#include <ext/superblock.h>
#include <ext/bfsext_export.h>
#include <ext/cache/directory.h>
#include <ext/cache/inode.h>

/**
 * @brief Init ext fs
 *
 * @param fs
 * @param bdev
 * @param read_only
 * @return int
 */
BFSEXT_NO_EXPORT int ext_fs_init(
  ext_fs_t* fs,
  common_blockdev_t* bdev,
  const bool read_only
) {
  // validate
  if ( ! fs || ! bdev ) {
    return EINVAL;
  }
  // populate fs information
  fs->bdev = bdev;
  fs->read_only = read_only;
  // populate inode cache
  fs->inode = ext_cache_inode_setup( fs );
  if ( ! fs->inode ) {
    return ENOMEM;
  }
  // populate directory cache
  fs->directory = ext_cache_directory_entry_setup( fs );
  if ( ! fs->directory ) {
    ext_cache_inode_destroy( fs->inode );
    return ENOMEM;
  }
  // read superblock into structure
  int result = ext_superblock_read( fs, &fs->superblock );
  if ( EOK != result ) {
    ext_cache_directory_entry_destroy( fs->directory );
    ext_cache_inode_destroy( fs->inode );
    return result;
  }
  // check superblock
  result = ext_superblock_check( fs );
  if ( EOK != result ) {
    ext_cache_directory_entry_destroy( fs->directory );
    ext_cache_inode_destroy( fs->inode );
    return result;
  }
  // get block size
  uint64_t bsize;
  result = ext_superblock_block_size( fs, &bsize );
  if ( EOK != result ) {
    ext_cache_directory_entry_destroy( fs->directory );
    ext_cache_inode_destroy( fs->inode );
    return result;
  }
  // check features
  result = ext_fs_check_feature( fs );
  if ( EOK != result ) {
    ext_cache_directory_entry_destroy( fs->directory );
    ext_cache_inode_destroy( fs->inode );
    return result;
  }
  // resize blocks
  result =  bdev->bdif->resize( bdev, bsize );
  if ( EOK != result ) {
    ext_cache_directory_entry_destroy( fs->directory );
    ext_cache_inode_destroy( fs->inode );
    return result;
  }
  /// FIXME: MOVE LOAD OF GROUPS INTO group.c => BEGIN
  // read total block groups
  uint64_t num_groups;
  ext_superblock_total_group_by_blocks( fs, &num_groups );
  // calculate total bytes
  const uint64_t total_bytes = num_groups * sizeof( ext_structure_block_group_descriptor_t );
  // calculate blocks to read
  const uint64_t blocks_to_read = ( total_bytes + bsize - 1 ) / bsize;
  // allocate space for groups
  fs->groups = malloc( ( size_t )( blocks_to_read * bsize ) );
  // handle error
  if ( ! fs->groups ) {
    ext_cache_directory_entry_destroy( fs->directory );
    ext_cache_inode_destroy( fs->inode );
    return ENOMEM;
  }
  // clear out
  memset( fs->groups, 0, ( size_t )( blocks_to_read * bsize ) );
  // read groups into array
  result = common_blockdev_bytes_read(
    fs->bdev,
    bsize == 1024 ? 2 : 1 * bsize,
    fs->groups,
    blocks_to_read * bsize
  );
  // handle error
  if ( EOK != result ) {
    ext_cache_directory_entry_destroy( fs->directory );
    ext_cache_inode_destroy( fs->inode );
    free( fs->groups );
    return result;
  }
  /// FIXME: MOVE LOAD OF GROUPS INTO group.c => END
  // resize block device buffer
  return EOK;
}

/**
 * @brief Method to destroy file system object
 *
 * @param fs
 * @return int
 */
BFSEXT_NO_EXPORT int ext_fs_fini( ext_fs_t* fs ) {
  if ( fs ) {
    // free all groups
    if ( fs->groups ) {
      free( fs->groups );
    }
    // free fs
    free( fs );
  }
  return EOK;
}

/**
 * @brief Check features
 *
 * @param fs
 * @return int
 */
BFSEXT_NO_EXPORT int ext_fs_check_feature( ext_fs_t* fs ) {
  ( void )fs;
  return EOK;
}
