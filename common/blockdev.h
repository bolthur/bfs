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

/**
 * @file common/blockdev.h
 */

#include <stdint.h>
#include <limits.h>
#include <stddef.h>

#include <common/sys/queue.h>

#ifndef _COMMON_BLOCKDEV_H
#define _COMMON_BLOCKDEV_H

#ifdef __cplusplus
extern "C" {
#endif

struct common_blockdev_iface;
typedef struct common_blockdev_iface common_blockdev_iface_t;

/** @brief Common block device structure */
typedef struct {
  /** @brief Block device interface */
  common_blockdev_iface_t* bdif;
  /** @brief Partition offset */
  uint64_t part_offset;
  /** @brief Total partition size */
  uint64_t part_size;
  /** @brief Pointer to file system structure */
  void* fs;
} common_blockdev_t;

/** @brief Common block device interface structure */
typedef struct common_blockdev_iface {
  /**
   * @brief Opens the block device
   * @param bdev device to open
   * @return int
   */
  int ( *open )( common_blockdev_t* bdev );
  /**
   * @brief Reads from block dev
   * @param bdev device to use
   * @param buf buffer to read into
   * @param blk_id block id to read
   * @param blk_cnt amount of subsequent blocks to read
   * @return int
   */
  int ( *read )( common_blockdev_t *bdev, void *buf, uint64_t blk_id, uint64_t blk_cnt );
  /**
   * @brief Writes to block dev
   * @param bdev device to use
   * @param buf buffer to write from
   * @param blk_id block id to start writing
   * @param blk_cnt amount of subsequent blocks to write
   * @return int
   */
  int ( *write )( common_blockdev_t *bdev, const void *buf, uint64_t blk_id, uint64_t blk_cnt );
  /**
   * @brief Close block device
   * @param bdev device to open
   * @return int
   */
  int ( *close )( common_blockdev_t* bdev );
  /**
   * @brief Lock device
   * @param bdev device to perform lock on
   * @return int
   */
  int ( *lock )( common_blockdev_t* bdev );
  /**
   * @brief Unock device
   * @param bdev device to perform unlock on
   * @return int
   */
  int ( *unlock )( common_blockdev_t* bdev );
  /**
   * @brief resize block buffer
   * @param bdev device to perform buffer resize
   * @param block_size new block size
   * @return int
   */
  int ( *resize )( common_blockdev_t* bdev, uint64_t block_size );
  /** @brief Size of one block in bytes */
  uint64_t block_size;
  /** @brief Internal block count */
  uint64_t block_count;
  /** @brief Internal block buffer */
  uint8_t* block_buffer;
  /** @brief Counter containing amount of started block dev inits */
  uint64_t reference_counter;
  /** @brief Counter for read operations */
  uint64_t read_counter;
  /** @brief Counter for write operations */
  uint64_t write_counter;
  /** @brief File name used for interaction */
  const char* filename;
  /** @brief Optional user data */
  void* p_user;
} common_blockdev_iface_t;

/** @brief Macro that can be used for single static blockdev instance */
#define COMMON_BLOCKDEV_STATIC_INSTANCE(_name, _bsize, _open, _read, _write, _close, _lock, _unlock, _resize ) \
  static uint8_t _name##_block_buffer[(_bsize)]; \
  static common_blockdev_iface_t _name##_iface = { \
		.open = _open, \
		.read = _read, \
		.write = _write, \
		.close = _close, \
		.lock = _lock, \
		.unlock = _unlock, \
    .resize = _resize, \
    .block_size = _bsize, \
    .block_count = 0, \
    .block_buffer = _name##_block_buffer, \
    .reference_counter = 0, \
    .read_counter = 0, \
    .write_counter = 0, \
    .filename = NULL, \
    .p_user = NULL, \
  }; \
  static common_blockdev_t _name = { \
    .bdif = &_name##_iface, \
    .part_offset = 0, \
    .part_size = 0, \
  }

#if defined( _BFS_COMPILING )
  typedef struct common_blockdev_entry {
    common_blockdev_t* bdev;
    char device_name[ PATH_MAX ];
    LIST_ENTRY( common_blockdev_entry ) list;
  } common_blockdev_entry_t;

  void common_blockdev_constructor( void ) __attribute__((constructor));
  void common_blockdev_destructor( void ) __attribute__((destructor));
  common_blockdev_t* common_blockdev_get_by_device_name( const char* device_name );

  void common_blockdev_if_lock( common_blockdev_t* bdev );
  void common_blockdev_if_unlock( common_blockdev_t* bdev );
  int common_blockdev_if_bytes_read( common_blockdev_t* bdev, void* buf, uint64_t block_id, uint64_t block_count );
  int common_blockdev_if_bytes_write( common_blockdev_t* bdev, void* buf, uint64_t block_id, uint64_t block_count );
#endif

int common_blockdev_register_device( common_blockdev_t* bdev, const char* device_name );
int common_blockdev_unregister_device( const char* device_name );
void common_blockdev_unregister_all( void );

int common_blockdev_init( common_blockdev_t* bdev );
int common_blockdev_fini( common_blockdev_t* bdev );

int common_blockdev_bytes_write( common_blockdev_t* bdev, uint64_t off, void* buf, uint64_t len );
int common_blockdev_bytes_read( common_blockdev_t* bdev, uint64_t off, void* buf, uint64_t len );

#ifdef __cplusplus
}
#endif

#endif
