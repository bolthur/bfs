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

/** @file ext/cache.h */

#include <stdint.h>
#include <ext/fs.h>

#ifndef _EXT_CACHE_H
#define _EXT_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief direntry hash size */
#define EXT_CACHE_DENTRY_HASH_SIZE 2048
/** @brief inode hash size */
#define EXT_CACHE_INODE_HASH_SIZE 2048
/** @brief inode hash mask */
#define EXT_CACHE_INODE_HASH_MASK 2047
/** @brief golden ratio */
#define EXT_CACHE_GOLDEN_RATIO 2654435761U

/** @brief inode cache entry */
typedef struct ext_cache_inode_node {
  /** @brief inode number */
  uint32_t inode_number;
  /** @brief size */
  uint32_t size;
  /** @brief mode */
  uint32_t mode;
  /** @brief block pointers */
  uint32_t block_pointer[ 15 ];
  /** @brief reference count */
  int reference_count;
  /** @brief dirty flag */
  bool dirty;
  /** @brief lru previous */
  struct ext_cache_inode_node *lru_prev;
  /** @brief lru next */
  struct ext_cache_inode_node *lru_next;
  /** @brief hash next */
  struct ext_cache_inode_node *hash_next;
} ext_cache_inode_node_t;

/** @brief inode cache structure */
typedef struct ext_cache_inode {
  /** @brief cache capacity */
  size_t capacity;
  /** @brief cache size */
  size_t size;
  /** @brief pointer to head */
  ext_cache_inode_node_t* head;
  /** @brief pointer to tail */
  ext_cache_inode_node_t* tail;
  /** @brief hash lookup table */
  ext_cache_inode_node_t* lookup[ EXT_CACHE_INODE_HASH_SIZE ];
} ext_cache_inode_t;

/** @brief dentry cache */
typedef struct ext_cache_directory_entry_node {
  /** @brief name */
  char* name;
  /** @brief pointer to parent dir */
  struct ext_cache_directory_entry_node* parent;
  /** @brief inode cache */
  ext_cache_inode_node_t* inode;
  /** @brief lru previous */
  struct ext_cache_directory_entry_node* lru_prev;
  /** @brief lru next */
  struct ext_cache_directory_entry_node* lru_next;
  /** @brief hash next */
  struct ext_cache_directory_entry_node* hash_next;
} ext_cache_directory_entry_node_t;

/** @brief directory entry cache structure */
typedef struct ext_cache_directory_entry {
  /** @brief cache capacity */
  size_t capacity;
  /** @brief cache size */
  size_t size;
  /** @brief pointer to head */
  ext_cache_directory_entry_node_t* head;
  /** @brief pointer to tail */
  ext_cache_directory_entry_node_t* tail;
  /** @brief hash lookup table */
  ext_cache_directory_entry_node_t* lookup[ EXT_CACHE_DENTRY_HASH_SIZE ];
} ext_cache_directory_entry_t;


#if defined( _BFS_COMPILING )
  // helper functions
  uint32_t ext_cache_hash_dentry( ext_cache_directory_entry_node_t*, const char* );
  uint32_t ext_cache_hash_inode( ext_fs_t*, uint32_t );

  // regular directory cache functions
  ext_cache_directory_entry_t* ext_cache_directory_entry_setup( void );
  void ext_cache_directory_entry_destroy( ext_cache_directory_entry_t* );
  void ext_cache_directory_entry_detach( ext_cache_directory_entry_t*, ext_cache_directory_entry_node_t* );
  void ext_cache_directory_entry_mode_to_head( ext_cache_directory_entry_t*, ext_cache_directory_entry_node_t* );

  // regular inode cache functions
  ext_cache_inode_t* ext_cache_inode_setup( void );
  void ext_cache_inode_destroy( ext_cache_inode_t* );
  void ext_cache_inode_detach( ext_cache_inode_t*, ext_cache_inode_node_t* );
  void ext_cache_inode_mode_to_head( ext_cache_inode_t*, ext_cache_inode_node_t* );
  void ext_cache_inode_drain( ext_cache_inode_t* );
#endif

#ifdef __cplusplus
}
#endif

#endif
