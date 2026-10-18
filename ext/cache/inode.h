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

/** @file ext/cache/inode.h */

#ifndef EXT_CACHE_INODE_H
#define EXT_CACHE_INODE_H

#include <stdint.h>
#include <ext/fs.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief inode hash size */
#define EXT_CACHE_INODE_HASH_SIZE 2048
/** @brief inode hash mask */
#define EXT_CACHE_INODE_HASH_MASK 2047
/** @brief golden ratio */
#define EXT_CACHE_GOLDEN_RATIO 2654435761U

/** @brief inode cache entry */
typedef struct ext_cache_inode_node {
  /** @brief inode number */
  uint64_t inode_number;
  /** @brief inode structure */
  ext_structure_inode_t inode;
  /** @brief reference count */
  int reference_count;
  /** @brief dirty flag */
  bool dirty;
  /** @brief lru previous */
  struct ext_cache_inode_node *lru_prev;
  /** @brief lru next */
  struct ext_cache_inode_node *lru_next;
  /** @brief hash previous */
  struct ext_cache_inode_node *hash_prev;
  /** @brief hash next */
  struct ext_cache_inode_node *hash_next;
} ext_cache_inode_node_t;

/** @brief inode cache structure */
typedef struct ext_cache_inode {
  /** @brief cache capacity */
  size_t capacity;
  /** @brief cache size */
  size_t size;
  /** @brief file system object */
  ext_fs_t* fs;
  /** @brief pointer to head */
  ext_cache_inode_node_t* head;
  /** @brief pointer to tail */
  ext_cache_inode_node_t* tail;
  /** @brief hash lookup table */
  ext_cache_inode_node_t* lookup[ EXT_CACHE_INODE_HASH_SIZE ];
} ext_cache_inode_t;

#if defined( _BFS_COMPILING )
  // helper functions
  uint64_t ext_cache_hash_inode( ext_fs_t*, uint64_t );
  // regular inode cache functions
  ext_cache_inode_t* ext_cache_inode_setup( ext_fs_t* );
  void ext_cache_inode_destroy( ext_cache_inode_t* );
  void ext_cache_inode_detach( ext_cache_inode_t*, ext_cache_inode_node_t* );
  void ext_cache_inode_move_to_head( ext_cache_inode_t*, ext_cache_inode_node_t* );
  void ext_cache_inode_drain( ext_cache_inode_t* );
  ext_cache_inode_node_t* ext_cache_inode_get( ext_cache_inode_t*, uint64_t );
  int ext_cache_inode_write( ext_cache_inode_t*, uint64_t, const uint8_t* );
  int ext_cache_inode_read( ext_cache_inode_t*, uint64_t, uint8_t* );
  ext_cache_inode_node_t* ext_cache_inode_lookup( ext_cache_inode_t*, uint64_t );
  void ext_cache_inode_insert( ext_cache_inode_t*, ext_cache_inode_node_t* );
  void ext_cache_inode_hash_remove( ext_cache_inode_t*, const ext_cache_inode_node_t* );
  ext_cache_inode_node_t* ext_cache_inode_allocate_or_evict( ext_cache_inode_t* );
  void ext_cache_inode_flush( ext_cache_inode_t*, ext_cache_inode_node_t* );
  void ext_cache_inode_put( ext_cache_inode_t*, uint64_t );
#endif

#ifdef __cplusplus
}
#endif

#endif
