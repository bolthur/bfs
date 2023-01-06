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

// IWYU pragma: no_include <errno.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <common/sys/queue.h>
#include <common/errno.h>
#include <common/blockdev.h>
#include <common/bfscommon_export.h>

static LIST_HEAD( common_blockdev_list, common_blockdev_entry ) device_list;

/**
 * @brief blockdev management constructor
 * @todo replace __attribute__ by some cross platform define
 */
BFSCOMMON_NO_EXPORT __attribute__((constructor)) void common_blockdev_constructor( void ) {
  LIST_INIT( &device_list );
}

/**
 * @brief blockdev management destructor
 * @todo replace __attribute__ by some cross platform define
 */
BFSCOMMON_NO_EXPORT __attribute__((destructor)) void common_blockdev_destructor( void ) {
  common_blockdev_unregister_all();
}

/**
 * @brief Get blockdev by device name
 *
 * @param device
 * @return common_blockdev_t*
 */
BFSCOMMON_NO_EXPORT common_blockdev_t* common_blockdev_get_by_device_name( const char* device_name ) {
  size_t device_name_length = strlen( device_name );
  // loop through list and check if device name already exists
  common_blockdev_entry_t* entry;
  LIST_FOREACH( entry, &device_list, list ) {
    if (
      strlen( entry->device_name ) == device_name_length
      && 0 == strcmp( device_name, entry->device_name )
    ) {
      return entry->bdev;
    }
  }
  // not found, so return null
  return NULL;
}

/**
 * @brief Method to register a device
 *
 * @param bdev
 * @param device_name
 * @return int
 */
BFSCOMMON_EXPORT int common_blockdev_register_device( common_blockdev_t* bdev, const char* device_name ) {
  // check for valid parameters
  if ( ! bdev || ! device_name ) {
    return EINVAL;
  }
  // handle path longer than allowed
  if ( strlen( device_name ) > PATH_MAX ) {
    return EINVAL;
  }
  // check if already added
  if ( common_blockdev_get_by_device_name( device_name ) ) {
    return EEXIST;
  }
  // allocate new entry
  common_blockdev_entry_t* entry = malloc( sizeof( *entry ) );
  if ( ! entry ) {
    return ENOMEM;
  }
  // clear out memory
  memset( entry, 0, sizeof( *entry ) );
  // populate things into
  entry->bdev = bdev;
  strcpy( entry->device_name, device_name );
  // insert into list
  LIST_INSERT_HEAD( &device_list, entry, list );
  // return success
  return EOK;
}

/**
 * @brief Method to unregister device by name
 *
 * @param device_name
 * @return int
 */
BFSCOMMON_EXPORT int common_blockdev_unregister_device( const char* device_name ) {
  // check parameter
  if ( ! device_name ) {
    return EINVAL;
  }
  size_t device_name_length = strlen( device_name );
  // loop through list try to find entry
  common_blockdev_entry_t* entry;
  LIST_FOREACH( entry, &device_list, list ) {
    if (
      strlen( entry->device_name ) == device_name_length
      && 0 == strcmp( device_name, entry->device_name )
    ) {
      // remove entry from list
      LIST_REMOVE( entry, list );
      // free entry
      free( entry );
      // return success
      return EOK;
    }
  }
  // return no entry found
  return ENOENT;
}

/**
 * @brief Method to unregister all block devices
 *
 * @return int
 */
BFSCOMMON_EXPORT void common_blockdev_unregister_all( void ) {
  // clear list by freeing up head until no head is existing any longer
  common_blockdev_entry_t* entry = device_list.lh_first;
  while( entry ) {
    // cache next element
    common_blockdev_entry_t* tmp = entry->list.le_next;
    // remove entry from list
    LIST_REMOVE( entry, list );
    // free everything
    free( entry );
    // overwrite entry with next
    entry = tmp;
  }
}

/**
 * @brief Init fat blockdev
 *
 * @param bdev
 * @return int
 */
BFSCOMMON_EXPORT int common_blockdev_init( common_blockdev_t* bdev ) {
  // validate parameter
  if (
    ! bdev
    || ! bdev->bdif
    || ! bdev->bdif->open
    || ! bdev->bdif->close
    || ! bdev->bdif->read
    || ! bdev->bdif->write
  ) {
    return EINVAL;
  }
  // increment reference counter if not 0 and return
  if ( bdev->bdif->reference_counter ) {
    bdev->bdif->reference_counter++;
    return EOK;
  }
  // open via lowlevel blockdevice
  int result = bdev->bdif->open( bdev );
  if ( result != EOK ) {
    return result;
  }
  // initialize reference counter
  bdev->bdif->reference_counter = 1;
  // return success
  return EOK;
}

/**
 * @brief Finish fat blockdev
 *
 * @param bdev
 * @return int
 */
BFSCOMMON_EXPORT int common_blockdev_fini( common_blockdev_t* bdev ) {
  // validate parameter
  if (
    ! bdev
    || ! bdev->bdif
    || ! bdev->bdif->open
    || ! bdev->bdif->close
    || ! bdev->bdif->read
    || ! bdev->bdif->write
  ) {
    return EINVAL;
  }
  // handle reference counter 0
  if ( ! bdev->bdif->reference_counter ) {
    return EOK;
  }
  // decrement
  bdev->bdif->reference_counter--;
  // stop if there is still something running
  if ( bdev->bdif->reference_counter ) {
    return EOK;
  }
  // close via lowlevel blockdevice
  return bdev->bdif->close( bdev );
}

/**
 * @brief Write bytes to blockdevice
 *
 * @param bdev
 * @param off
 * @param buf
 * @param len
 * @return int
 */
BFSCOMMON_EXPORT int common_blockdev_bytes_write(
  common_blockdev_t* bdev,
  uint64_t off,
  void* buf,
  size_t len
) {
  ( void )bdev,
  ( void )off,
  ( void )buf,
  ( void )len;
  return ENOSYS;
}

/**
 * @brief Read bytes from block device
 *
 * @param bdev
 * @param off
 * @param buf
 * @param len
 * @return int
 */
BFSCOMMON_EXPORT int common_blockdev_bytes_read(
  common_blockdev_t* bdev,
  uint64_t off,
  void* buf,
  size_t len
) {
  // check parameters
  if ( ! bdev || ! buf ) {
    return EINVAL;
  }
  // handle not opened
  if ( ! bdev->bdif->reference_counter ) {
    return EIO;
  }
  // handle out of range
  if ( ( off + len ) > bdev->part_size ) {
    return EINVAL;
  }
  // calculate block index
  uint64_t block_index = ( off + bdev->part_offset ) / bdev->bdif->block_size;
  uint8_t* buffer = buf;
  int result;
  // check whether start is misaligned block
  size_t unaligned = ( size_t )( off & ( bdev->bdif->block_size - 1 ) );
  if ( unaligned ) {
    // determine read length
    size_t rlen = len;
    if ( len > bdev->bdif->block_size - unaligned ) {
      rlen = bdev->bdif->block_size - unaligned;
    }
    // read whole block index
    result = common_blockdev_if_bytes_read(
      bdev, bdev->bdif->block_buffer, block_index, 1
    );
    if ( EOK != result ) {
      return result;
    }
    // copy over partially
    memcpy( buffer, bdev->bdif->block_buffer + unaligned, rlen );
    // increase pointer, decrease length and increase block index
    buffer += rlen;
    len -= rlen;
    block_index++;
  }
  // continue with aligned data
  size_t blen = len / bdev->bdif->block_size;
  if ( blen ) {
    // read block index
    result = common_blockdev_if_bytes_read(
      bdev, buffer, block_index, blen
    );
    if ( EOK != result ) {
      return result;
    }
    // increment buffer and decrement length
    size_t read_length = bdev->bdif->block_size * blen;
    buffer += read_length;
    len -= read_length;
    block_index += blen;
  }
  // remaining data
  if ( len ) {
    // read remaining block
    result = common_blockdev_if_bytes_read(
      bdev, bdev->bdif->block_buffer, block_index, 1
    );
    if ( EOK != result ) {
      return result;
    }
    // copy over content
    memcpy( buffer, bdev->bdif->block_buffer, len );
  }
  // return success
  return EOK;
}

/**
 * @brief Wrapper to call interface lock function
 *
 * @param bdev
 */
BFSCOMMON_NO_EXPORT void common_blockdev_if_lock( common_blockdev_t* bdev ) {
  if ( ! bdev || ! bdev->bdif->lock ) {
    return;
  }
  assert( EOK == bdev->bdif->lock( bdev ) );
}

/**
 * @brief Wrapper to call interface unlock function
 *
 * @param bdev
 */
BFSCOMMON_NO_EXPORT void common_blockdev_if_unlock( common_blockdev_t* bdev ) {
  if ( ! bdev || ! bdev->bdif->unlock ) {
    return;
  }
  assert( EOK == bdev->bdif->unlock( bdev ) );
}

/**
 * @brief Wrapper around interface read function
 *
 * @param bdev
 * @param buf
 * @param block_id
 * @param block_count
 * @return int
 */
BFSCOMMON_NO_EXPORT int common_blockdev_if_bytes_read(
  common_blockdev_t* bdev,
  void* buf,
  uint64_t block_id,
  size_t block_count
) {
  common_blockdev_if_lock( bdev );
  int result = bdev->bdif->read( bdev, buf, block_id, block_count );
  bdev->bdif->read_counter++;
  common_blockdev_if_unlock( bdev );
  return result;
}

/**
 * @brief Wrapper around interface write function
 *
 * @param bdev
 * @param buf
 * @param block_id
 * @param block_count
 * @return int
 */
BFSCOMMON_NO_EXPORT int common_blockdev_if_bytes_write(
  common_blockdev_t* bdev,
  void* buf,
  uint64_t block_id,
  size_t block_count
) {
  common_blockdev_if_lock( bdev );
  int result = bdev->bdif->read( bdev, buf, block_id, block_count );
  bdev->bdif->read_counter++;
  common_blockdev_if_unlock( bdev );
  return result;
}
