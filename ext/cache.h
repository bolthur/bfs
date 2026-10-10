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
typedef struct cache_inode {
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
  struct cache_inode *lru_prev;
  /** @brief lru next */
  struct cache_inode *lru_next;
  /** @brief hash next */
  struct cache_inode *hash_next;
} cache_inode_t;

/** @brief dentry cache */
typedef struct directory_entry_cache {
  /** @brief name */
  char* name;
  /** @brief pointer to parent dir */
  struct directory_entry_cache* parent;
  /** @brief inode cache */
  cache_inode_t* inode;
  /** @brief lru previous */
  struct directory_entry_cache* lru_prev;
  /** @brief lru next */
  struct directory_entry_cache* lru_next;
  /** @brief hash next */
  struct directory_entry_cache* hash_next;
} directory_entry_cache_t;

#if defined( _BFS_COMPILING )
  uint32_t ext_cache_hash_dentry( directory_entry_cache_t*, const char* );
  uint32_t ext_cache_hash_inode( ext_fs_t*, uint32_t );
#endif

#ifdef __cplusplus
}
#endif

#endif
