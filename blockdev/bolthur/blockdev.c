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

// stuff for open, close, read, write
#include <unistd.h>
#include <fcntl.h>
// generic stuff
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>

// library specific stuff
#include <common/blockdev.h>
#include <common/errno.h>
#include <blockdev/bfsblockdev_export.h>

// local stuff
#include "blockdev.h"

/**
 * @brief Method to get handle from user data
 *
 * @param bdev
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_get_handle( common_blockdev_t* bdev ) {
  int handle = 0;
  memcpy( &handle, bdev->bdif->p_user, sizeof( int ) );
  return handle;
}

/**
 * @brief Method to set handle within user information
 *
 * @param bdev
 * @param new_handle
 */
BFSBLOCKDEV_NO_EXPORT void blockdev_set_handle( common_blockdev_t* bdev, int new_handle ) {
  memcpy( bdev->bdif->p_user, &new_handle, sizeof( int ) );
}

/**
 * @brief Method to open block device
 *
 * @param bdev
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_open( common_blockdev_t* bdev ) {
  // handle not yet set
  if ( !bdev->bdif->filename ) {
    return ENODATA;
  }
  // fetch stat information
  struct stat st;
  if ( -1 == stat( bdev->bdif->filename, &st ) ) {
    return EIO;
  }
  // open file
  int dev_file = open( bdev->bdif->filename, O_RDWR );
  if ( -1 == dev_file ) {
    return EIO;
  }
  // populate bunch of information from stat result
  bdev->part_size = ( uint32_t )st.st_size;
  bdev->bdif->block_count = bdev->part_size / bdev->bdif->block_size;
  // allocate space for file dscriptor
  if ( ! bdev->bdif->p_user ) {
    void* p_user = malloc( sizeof( int ) );
    if ( ! p_user ) {
      close( dev_file );
      return EIO;
    }
    // set p_user
    bdev->bdif->p_user = p_user;
  }
  // set handle
  blockdev_set_handle( bdev, dev_file );
  // return success
  return EOK;
}

/**
 * @brief Method to read from block device
 *
 * @param bdev
 * @param buf
 * @param blk_id
 * @param blk_cnt
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_read(
  common_blockdev_t* bdev,
  void* buf,
  uint64_t blk_id,
  size_t blk_cnt
) {
  if ( ! blk_cnt ) {
    return EOK;
  }
  // calculate size and offset
  size_t size = bdev->bdif->block_size * blk_cnt;
  off_t offset = ( off_t )blk_id * ( off_t )bdev->bdif->block_size;
  // read from device
  ssize_t result = pread( blockdev_get_handle( bdev ), buf, size, offset );
  // handle error
  if ( -1 == result || ( size_t )result != size ) {
    return EIO;
  }
  // return success
  return EOK;
}

/**
 * @brief Method to write to block device
 *
 * @param bdev
 * @param buf
 * @param blk_id
 * @param blk_cnt
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_write(
  common_blockdev_t* bdev,
  const void* buf,
  uint64_t blk_id,
  size_t blk_cnt
) {
  if ( ! blk_cnt ) {
    return EOK;
  }
  // calculate size and offset
  size_t size = bdev->bdif->block_size * blk_cnt;
  off_t offset = ( off_t )blk_id * ( off_t )bdev->bdif->block_size;
  // read from device
  ssize_t result = pwrite( blockdev_get_handle( bdev ), buf, size, offset );
  // handle error
  if ( -1 == result || ( size_t )result != size ) {
    return EIO;
  }
  // return success
  return EOK;
}

/**
 * @brief Close block device
 *
 * @param bdev
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_close( common_blockdev_t* bdev ) {
  int fd = blockdev_get_handle( bdev );
  if ( fd ) {
    close( fd );
    blockdev_set_handle( bdev, 0 );
  }
  return EOK;
}

/**
 * @brief Lock block device
 *
 * @param bdev
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_lock( common_blockdev_t* bdev ) {
  ( void )bdev;
  return EOK;
}

/**
 * @brief Unlock block device
 *
 * @param bdev
 * @return int
 */
BFSBLOCKDEV_NO_EXPORT int blockdev_unlock( common_blockdev_t* bdev ) {
  ( void )bdev;
  return EOK;
}

/**
 * @brief Get new block device
 *
 * @param filename
 * @return common_blockdev_t*
 */
BFSBLOCKDEV_EXPORT common_blockdev_t* common_blockdev_get( const char* filename ) {
  // allocate block device
  common_blockdev_t* bdev = malloc( sizeof( *bdev ) );
  // handle error
  if ( ! bdev ) {
    return NULL;
  }
  // allocate substructure
  common_blockdev_iface_t* bdif = malloc( sizeof( *bdif ) );
  // handle error
  if ( ! bdif ) {
    free( bdev );
    return NULL;
  }
  // allocate buffer
  uint8_t* buffer = malloc( sizeof( *buffer ) * 512 );
  // handle error
  if ( ! buffer ) {
    free( bdif );
    free( bdev );
    return NULL;
  }
  // allocate space for user data
  void* p_user = malloc( sizeof( int ) );
  if ( ! p_user ) {
    free( buffer );
    free( bdif );
    free( bdev );
    return NULL;
  }
  // duplicate name into temporary
  const char* tmp_filename = strdup( filename );
  // handle error
  if ( ! tmp_filename ) {
    free( p_user );
    free( buffer );
    free( bdif );
    free( bdev );
    return NULL;
  }
  // clear memory
  memset( bdev, 0, sizeof( *bdev ) );
  memset( bdif, 0, sizeof( *bdif ) );
  memset( buffer, 0, sizeof( *buffer ) * 512 );
  // populate interface
  bdif->open = blockdev_open;
  bdif->read = blockdev_read;
  bdif->write = blockdev_write;
  bdif->close = blockdev_close;
  bdif->lock = blockdev_lock;
  bdif->unlock = blockdev_unlock;
  bdif->block_size = 512;
  bdif->block_count = 0;
  bdif->block_buffer = buffer;
  // populate device
  bdev->bdif = bdif;
  bdev->part_offset = 0;
  bdev->part_size = 0;
  bdev->bdif->read_counter = 0;
  bdev->bdif->write_counter = 0;
  bdev->bdif->filename = tmp_filename;
  bdev->bdif->p_user = p_user;
  // reset file descriptor
  blockdev_set_handle( bdev, 0 );
  // return device
  return bdev;
}

/**
 * @brief destruct blockdev
 *
 * @param bdev
 * @return int
 */
BFSBLOCKDEV_EXPORT int common_blockdev_destruct( common_blockdev_t* bdev ) {
  // validate parameter
  if ( ! bdev ) {
    return EINVAL;
  }
  // check for file is closed
  if ( blockdev_get_handle( bdev ) ) {
    return EIO;
  }
  // check if fs is set ( indicator that it's still in use )
  if ( bdev->fs ) {
    return EIO;
  }
  // free dynamically allocated data structures
  if ( bdev->p_user ) {
    free( bdev->p_user );
  }
  if ( bdev->buffer ) {
    free( bdev->buffer );
  }
  if ( bdev->bdif ) {
    free( bdev->bdif );
  }
  free( bdev );
}
