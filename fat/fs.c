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
#include <fat/fs.h>
#include <fat/structure.h>
#include <fat/superblock.h>
#include <fat/type.h>
#include <fat/bfsfat_export.h>

/**
 * @brief Init fat fs
 *
 * @param fs
 * @param bdev
 * @param read_only
 * @return int
 */
BFSFAT_EXPORT int fat_fs_init( fat_fs_t* fs, common_blockdev_t* bdev, bool read_only ) {
  // validate
  if ( ! fs || ! bdev ) {
    return EINVAL;
  }
  // populate fs information
  fs->bdev = bdev;
  fs->read_only = read_only;
  // read superblock into structure
  int result = fat_superblock_read( fs->bdev, &fs->superblock );
  if ( EOK != result ) {
    return result;
  }
  // check superblock
  result = fat_superblock_check( fs );
  if ( EOK != result ) {
    return result;
  }
  // validate
  result = fat_type_validate( fs );
  if ( EOK != result ) {
    return result;
  }
  // resize block device buffer
  return bdev->bdif->resize( bdev, fs->superblock.bytes_per_sector );
}

/**
 * @brief Method to destroy file system object
 *
 * @param fs
 * @return int
 */
BFSFAT_NO_EXPORT int fat_fs_fini( fat_fs_t* fs ) {
  if ( fs ) {
    free( fs );
  }
  return EOK;
}
