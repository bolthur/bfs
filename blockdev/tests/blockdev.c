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
#include <stdio.h>
#include <stddef.h>
#include <common/errno.h>
#include <common/blockdev.h>
#include <blockdev/bfsblockdev_export.h>
#include "blockdev.h"

static const char *fname = NULL;
static FILE* fp = NULL;

// Generate static blockdev instance
COMMON_BLOCKDEV_STATIC_INSTANCE(
  blockdev,
  512,
  blockdev_open,
  blockdev_read,
  blockdev_write,
  blockdev_close,
  blockdev_lock,
  blockdev_unlock,
  blockdev_resize
);

/**
 * @brief Open block device
 *
 * @param bdev
 * @return int
 *
 * @private
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_open( common_blockdev_t* bdev ) {
  // handle not yet set
  if ( !fname ) {
    return ENODATA;
  }
  // try to open file
  fp = fopen( fname, "r+b" );
  if ( ! fp ) {
    return EIO;
  }
  // disable file buffering
  setbuf( fp, 0 );
  // try to get end of file
  if ( fseeko( fp, 0, SEEK_END ) ) {
    return EFAULT;
  }
  // populate bunch of information
  bdev->part_offset = 0;
  bdev->part_size = ( uint64_t )ftello( fp );
  bdev->bdif->block_count = bdev->part_size / bdev->bdif->block_size;
  // return success
  return EOK;
}

/**
 * @brief Read from block device
 *
 * @param bdev
 * @param buf
 * @param blk_id
 * @param blk_cnt
 * @return int
 *
 * @private
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_read(
  common_blockdev_t *bdev,
  void *buf,
  uint64_t blk_id,
  uint64_t blk_cnt
) {
  if ( ! blk_cnt ) {
    return EOK;
  }
  if ( fseeko( fp, ( off_t )( blk_id * bdev->bdif->block_size ), SEEK_SET ) ) {
    return EIO;
  }
  if ( ! fread( buf, bdev->bdif->block_size * blk_cnt, 1, fp ) ) {
    return EIO;
  }
  return EOK;
}

/**
 * @brief Write to block device
 *
 * @param bdev
 * @param buf
 * @param blk_id
 * @param blk_cnt
 * @return int
 *
 * @private
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_write(
  common_blockdev_t *bdev,
  const void *buf,
  uint64_t blk_id,
  uint64_t blk_cnt
) {
  if ( ! blk_cnt ) {
    return EOK;
  }
  if ( fseeko( fp, ( off_t )( blk_id * bdev->bdif->block_size ), SEEK_SET ) ) {
    return EIO;
  }
  if ( ! fwrite( buf, bdev->bdif->block_size * blk_cnt, 1, fp ) ) {
    return EIO;
  }
  return EOK;
}

/**
 * @brief Close block device
 *
 * @param bdev
 * @return int
 *
 * @private
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_close( common_blockdev_t *bdev ) {
  ( void )bdev;
  fclose( fp );
  return EOK;
}

/**
 * @brief Lock block device
 *
 * @param bdev
 * @return int
 *
 * @private
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_lock( common_blockdev_t *bdev ) {
  ( void )bdev;
  if ( 0 != ftrylockfile( fp ) ) {
    return EIO;
  }
  return EOK;
}

/**
 * @brief Unlock block device
 *
 * @param bdev
 * @return int
 *
 * @private
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_unlock( common_blockdev_t* bdev ) {
  ( void )bdev;
  funlockfile( fp );
  return EOK;
}

/**
 * @brief Resize block device buffer
 *
 * @param bdev
 * @param block_size
 * @return int
 *
 * @private
 */
int blockdev_resize( common_blockdev_t* bdev, uint64_t block_size ) {
  ( void )bdev;
  ( void )block_size;
  return EOK;
}

/**
 * @brief Get block device object
 *
 * @return common_blockdev_t*
 *
 * @public
 */
BFSBLOCKDEV_EXPORT common_blockdev_t *common_blockdev_get( void ) {
  return &blockdev;
}

/**
 * @brief Change filename of block device
 *
 * @param fname_new
 *
 * @public
 */
BFSBLOCKDEV_EXPORT void common_blockdev_set_fname( const char* fname_new ) {
  fname = fname_new;
}
