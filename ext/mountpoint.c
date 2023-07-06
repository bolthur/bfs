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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <common/errno.h> // IWYU pragma: keep
#include <common/blockdev.h>
#include <common/mountpoint.h>
#include <ext/fs.h>
#include <ext/mountpoint.h>
#include <ext/bfsext_export.h>
#include <bfsconfig.h>

/**
 * @brief Mount device
 *
 * @param device_name
 * @param mountpoint
 * @param read_only
 * @return int
 */
BFSEXT_EXPORT int ext_mountpoint_mount(
  const char* device_name,
  const char* mountpoint,
  bool read_only
) {
  // check parameter
  if ( ! device_name || ! mountpoint ) {
    return EINVAL;
  }
  // get mountpoint length
  size_t mountpoint_length = strlen( mountpoint );
  // validate size
  if ( mountpoint_length > PATH_MAX ) {
    return EINVAL;
  }
  // handle no folder mount
  if ( mountpoint[ mountpoint_length - 1 ] != CONFIG_PATH_SEPARATOR_CHAR ) {
    return ENOTSUP;
  }
  // loop through list try to find already mounted entry
  common_mountpoint_t* entry = common_mountpoint_by_mountpoint( mountpoint );
  if ( entry ) {
    return EOK;
  }
  // get block device
  common_blockdev_t* bdev = common_blockdev_get_by_device_name( device_name );
  // handle nothing found
  if ( ! bdev ) {
    return ENODEV;
  }

  // init for transfer
  int result = common_blockdev_init( bdev );
  if ( EOK != result ) {
    return result;
  }

  // allocate space for fs structure
  ext_fs_t* fs = malloc( sizeof( *fs ) );
  if ( ! fs ) {
    return ENOMEM;
  }
  // init fat fs
  result = ext_fs_init( fs, bdev, read_only );
  if ( EOK != result ) {
    free( fs );
    common_blockdev_fini( bdev );
    return result;
  }
  // add mountpoint
  result = common_mountpoint_add( mountpoint, fs, true, NULL );
  if ( EOK != result ) {
    free( fs );
    common_blockdev_fini( bdev );
    return result;
  }
  // set fs property of blockdevice
  bdev->fs = fs;
  // return success
  return EOK;
}

/**
 * @brief Unmount mountpoint
 *
 * @param mountpoint
 * @return int
 */
BFSEXT_EXPORT int ext_mountpoint_umount( const char* mountpoint ) {
  // loop through list try to find already mounted entry
  common_mountpoint_t* entry = common_mountpoint_by_mountpoint( mountpoint );
  if ( ! entry ) {
    return ENODEV;
  }
  // get fat fs instance
  ext_fs_t* fs = entry->fs;
  // finish
  int result = common_blockdev_fini( fs->bdev );
  if ( EOK != result ) {
    return result;
  }
  // set mounted to false
  entry->mounted = false;
  entry->fs = NULL;
  fs->bdev->fs = NULL;
  // cleanup
  result = common_mountpoint_remove( mountpoint );
  if ( EOK != result ) {
    return result;
  }
  // destroy fs
  result = ext_fs_fini( fs );
  if ( EOK != result ) {
    return result;
  }
  // return success
  return EOK;
}
