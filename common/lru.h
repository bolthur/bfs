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

/** @file common/lru.h */

#include <stdint.h>
#include <stdbool.h>
#include <common/blockdev.h>

#ifndef _COMMON_LRU_H
#define _COMMON_LRU_H

#ifdef __cplusplus
extern "C" {
#endif

#define LRU_CACHE_CAPACITY 1024

/** @brief common lru cache block */
typedef struct common_lru_cache_node {
  /** @brief block number */
  uint64_t block_number;
  /** @brief pointer to data */
  uint8_t* data;
  /** @brief dirty flag */
  bool dirty;
  /** @brief pointer to next element */
  struct common_lru_cache_node* next;
  /** @brief pointer to previous element */
  struct common_lru_cache_node* prev;
} common_lru_cache_node_t;

/** @brief lru cache */
typedef struct common_lru_cache {
  /** @brief cache capacity */
  size_t capacity;
  /** @brief cache size */
  size_t size;
  /** @brief node size */
  size_t node_data_size;
  /** @brief block device */
  common_blockdev_t* bdev;
  /** @brief pointer to head */
  common_lru_cache_node_t* head;
  /** @brief pointer to tail */
  common_lru_cache_node_t* tail;
  /** @brief hash lookup table */
  common_lru_cache_node_t* lookup[ LRU_CACHE_CAPACITY ];
} common_lru_cache_t;

#if defined( _BFS_COMPILING )
  void common_lru_destructor( void );

  common_lru_cache_t* common_lru_setup( common_blockdev_t*, size_t );
  void common_lru_destroy( common_lru_cache_t* );
  void common_lru_drain( common_lru_cache_t* );
  void common_lru_detach( common_lru_cache_t*, common_lru_cache_node_t* );
  void common_lru_move_to_head( common_lru_cache_t*, common_lru_cache_node_t* );
  common_lru_cache_node_t* common_lru_get( common_lru_cache_t*, uint64_t );
  int common_lru_write( common_lru_cache_t*, uint64_t, const uint8_t* );
  int common_lru_read( common_lru_cache_t*, uint64_t, uint8_t* );
#endif

#ifdef __cplusplus
}
#endif

#endif
