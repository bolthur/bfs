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
#include <common/errno.h>
#include <common/blockdev.h>
#include <ext/fs.h>
#include <ext/structure.h>
#include <ext/superblock.h>
#include <ext/bfsext_export.h>

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
  bool read_only
) {
  // validate
  if ( ! fs || ! bdev ) {
    return EINVAL;
  }
  // populate fs information
  fs->bdev = bdev;
  fs->read_only = read_only;
  // read superblock into structure
  int result = ext_superblock_read( fs, &fs->superblock );
  if ( EOK != result ) {
    return result;
  }
  // check superblock
  result = ext_superblock_check( fs );
  if ( EOK != result ) {
    return result;
  }
  // get block size
  uint64_t bsize;
  result = ext_superblock_block_size( fs, &bsize );
  if ( EOK != result ) {
    return result;
  }
  // check features
  result = ext_fs_check_feature( fs );
  if ( EOK != result ) {
    return result;
  }
  // resize block device buffer
  return bdev->bdif->resize( bdev, bsize );
}

/**
 * @brief Method to destroy file system object
 *
 * @param fs
 * @return int
 */
BFSEXT_NO_EXPORT int ext_fs_fini( ext_fs_t* fs ) {
  if ( fs ) {
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
