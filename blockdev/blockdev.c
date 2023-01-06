/**
 * Copyright (C) 2022 bolthur project.
 *
 * This file is part of bolthur/bfs.
 *
 * bolthur/bfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bolthur/bfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bolthur/bfs.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common/errno.h>
#include <blockdev/bfsblockdev_export.h>
#include "blockdev.h"

// Generate static blockdev instance
COMMON_BLOCKDEV_STATIC_INSTANCE(
  blockdev,
  512,
  blockdev_open,
  blockdev_read,
  blockdev_write,
  blockdev_close,
  blockdev_lock,
  blockdev_unlock
);

/**
 * @brief Open block device
 *
 * @param bdev
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_open( common_blockdev_t* bdev ) {
  ( void )bdev;
  return EIO;
}

/**
 * @brief Read from block device
 *
 * @param bdev
 * @param buf
 * @param blk_id
 * @param blk_cnt
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_read(
  common_blockdev_t *bdev,
  void *buf,
  uint64_t blk_id,
  size_t blk_cnt
) {
  ( void )bdev;
  ( void )buf;
  ( void )blk_id;
  ( void )blk_cnt;
  return EIO;
}

/**
 * @brief Write to block device
 *
 * @param bdev
 * @param buf
 * @param blk_id
 * @param blk_cnt
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_write(
  common_blockdev_t *bdev,
  const void *buf,
  uint64_t blk_id,
  size_t blk_cnt
) {
  ( void )bdev;
  ( void )buf;
  ( void )blk_id;
  ( void )blk_cnt;
  return EIO;
}

/**
 * @brief Close block device
 *
 * @param bdev
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_close( common_blockdev_t *bdev ) {
  ( void )bdev;
  return EIO;
}

/**
 * @brief Lock block device
 *
 * @param bdev
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_lock( common_blockdev_t *bdev ) {
  ( void )bdev;
  return EIO;
}

/**
 * @brief Unlock block device
 *
 * @param bdev
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_unlock( common_blockdev_t* bdev ) {
  ( void )bdev;
  return EIO;
}

/**
 * @brief Get block device object
 *
 * @return common_blockdev_t*
 */
BFSBLOCKDEV_EXPORT common_blockdev_t *common_blockdev_get( void ) {
  return &blockdev;
}
