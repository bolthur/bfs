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

typedef struct {
  common_blockdev_iface_t* bdif;
  uint64_t part_offset;
  uint64_t part_size;
  void* fs;
} common_blockdev_t;

typedef struct common_blockdev_iface {
  int ( *open )( common_blockdev_t* bdev );
  int ( *read )( common_blockdev_t *bdev, void *buf, uint64_t blk_id, size_t blk_cnt );
  int ( *write )( common_blockdev_t *bdev, const void *buf, uint64_t blk_id, size_t blk_cnt );
  int ( *close )( common_blockdev_t* bdev );
  int ( *lock )( common_blockdev_t* bdev );
  int ( *unlock )( common_blockdev_t* bdev );
  uint32_t block_size;
  uint64_t block_count;
  uint8_t* block_buffer;
  uint32_t reference_counter;
  uint32_t read_counter;
  uint32_t write_counter;
  const char* filename;
  void* p_user;
} common_blockdev_iface_t;

#define COMMON_BLOCKDEV_STATIC_INSTANCE(_name, _bsize, _open, _read, _write, _close, _lock, _unlock ) \
  static uint8_t _name##_block_buffer[(_bsize)]; \
  static common_blockdev_iface_t _name##_iface = { \
		.open = _open, \
		.read = _read, \
		.write = _write, \
		.close = _close, \
		.lock = _lock, \
		.unlock = _unlock, \
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
  int common_blockdev_if_bytes_read( common_blockdev_t* bdev, void* buf, uint64_t block_id, size_t block_count );
  int common_blockdev_if_bytes_write( common_blockdev_t* bdev, void* buf, uint64_t block_id, size_t block_count );
#endif

int common_blockdev_register_device( common_blockdev_t* bdev, const char* device_name );
int common_blockdev_unregister_device( const char* device_name );
void common_blockdev_unregister_all( void );

int common_blockdev_init( common_blockdev_t* bdev );
int common_blockdev_fini( common_blockdev_t* bdev );

int common_blockdev_bytes_write( common_blockdev_t* bdev, uint64_t off, void* buf, size_t len );
int common_blockdev_bytes_read( common_blockdev_t* bdev, uint64_t off, void* buf, size_t len );

#ifdef __cplusplus
}
#endif

#endif
