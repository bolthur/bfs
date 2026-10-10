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

#include <stdlib.h>
#include <string.h>
#include <common/bfscommon_export.h>
#include <ext/cache.h>

/**
 * Helper to create new dentry hash
 * @param parent parent cache entry
 * @param name dentry name
 * @return
 */
BFSCOMMON_NO_EXPORT uint32_t ext_cache_hash_dentry(
  ext_cache_directory_entry_node_t* parent,
  const char* name
) {
  uint32_t hash = ( uint32_t )( ( uintptr_t )parent );
  while ( *name ) {
    const uint32_t c = ( uint32_t )( *name++ );
    hash = ( hash * 31U ) + c;
  }
  return hash % EXT_CACHE_DENTRY_HASH_SIZE;
}

/**
 * Create hash of inode
 * @param fs file system object
 * @param inode_number inode number
 * @return
 */
BFSCOMMON_NO_EXPORT uint32_t ext_cache_hash_inode(
  ext_fs_t* fs,
  const uint32_t inode_number
) {
  const uint32_t mount_id = ( uint32_t )( ( uintptr_t )fs );
  const uint32_t hash = ( inode_number ^ mount_id ) * EXT_CACHE_GOLDEN_RATIO;
  return hash & EXT_CACHE_INODE_HASH_MASK;
}

/**
 * Setup directory entry cache
 * @return
 */
BFSCOMMON_NO_EXPORT ext_cache_directory_entry_t* ext_cache_directory_entry_setup( void ) {
  // allocate
  ext_cache_directory_entry_t* lru = malloc( sizeof( *lru ) );
  if ( ! lru ) {
    return nullptr;
  }
  // clear out
  memset( lru, 0, sizeof( *lru ) );
  // populate bare minimum
  lru->capacity = EXT_CACHE_DENTRY_HASH_SIZE;
  // return success
  return lru;
}

/**
 * Destroy directy entry cache
 * @param cache cache to destroy
 * @todo implement correctly
 */
BFSCOMMON_NO_EXPORT void ext_cache_directory_entry_destroy(
  [[maybe_unused]] ext_cache_directory_entry_t* cache
) {
}

/**
 * Detach inode
 * @param cache cache to operate on
 * @param node node to detach
 */
BFSCOMMON_NO_EXPORT void ext_cache_directory_entry_detach(
  ext_cache_directory_entry_t* cache,
  ext_cache_directory_entry_node_t* node
) {
  if ( ! cache || ! node ) {
    return;
  }
  if ( node->lru_prev ) {
    node->lru_prev->lru_next = node->lru_next;
  }
  if ( node->lru_next ) {
    node->lru_next->lru_prev = node->lru_prev;
  }
  if ( cache->head == node ) {
    cache->head = node->lru_next;
  }
  if ( cache->tail == node ) {
    cache->tail = node->lru_prev;
  }
  node->lru_next = node->lru_prev = nullptr;
}

/**
 * Move directory entry cache to head
 * @param cache cache to operate on
 * @param node node to move to head
 */
BFSCOMMON_NO_EXPORT void ext_cache_directory_entry_mode_to_head(
  ext_cache_directory_entry_t* cache,
  ext_cache_directory_entry_node_t* node
) {
  if ( ! cache || ! node ) {
    return;
  }
  ext_cache_directory_entry_detach( cache, node );
  node->lru_next = cache->head;
  if ( cache->head ) {
    cache->head->lru_next = node;
  }
  cache->head = node;
  if ( ! cache->tail ) {
    cache->tail = node;
  }
}

/**
 * Setup inode cache
 * @return
 */
BFSCOMMON_NO_EXPORT ext_cache_inode_t* ext_cache_inode_setup( void ) {
  // allocate
  ext_cache_inode_t* lru = malloc( sizeof( *lru ) );
  if ( ! lru ) {
    return nullptr;
  }
  // clear out
  memset( lru, 0, sizeof( *lru ) );
  // populate bare minimum
  lru->capacity = EXT_CACHE_INODE_HASH_SIZE;
  // return success
  return lru;
}

/**
 * Destroy inode cache
 * @param cache cache to destroy
 * @todo implement correctly
 */
BFSCOMMON_NO_EXPORT void ext_cache_inode_destroy(
  [[maybe_unused]] ext_cache_inode_t* cache
) {
}

/**
 * Detach inode
 * @param cache cache to operate on
 * @param node node to detach
 */
BFSCOMMON_NO_EXPORT void ext_cache_inode_detach(
  ext_cache_inode_t* cache,
  ext_cache_inode_node_t* node
) {
  if ( ! cache || ! node ) {
    return;
  }
  if ( node->lru_prev ) {
    node->lru_prev->lru_next = node->lru_next;
  }
  if ( node->lru_next ) {
    node->lru_next->lru_prev = node->lru_prev;
  }
  if ( cache->head == node ) {
    cache->head = node->lru_next;
  }
  if ( cache->tail == node ) {
    cache->tail = node->lru_prev;
  }
  node->lru_next = node->lru_prev = nullptr;
}

/**
 * Move inode entry cache to head
 * @param cache cache to operate on
 * @param node node to move to head
 */
BFSCOMMON_NO_EXPORT void ext_cache_inode_mode_to_head(
  ext_cache_inode_t* cache,
  ext_cache_inode_node_t* node
) {
  if ( ! cache || ! node ) {
    return;
  }
  ext_cache_inode_detach( cache, node );
  node->lru_next = cache->head;
  if ( cache->head ) {
    cache->head->lru_prev = node;
  }
  cache->head = node;
  if ( ! cache->tail ) {
    cache->tail = node;
  }
}

/**
 * Drain inode cache
 * @param cache cache to drain
 * @todo implement properly
 */
BFSCOMMON_NO_EXPORT void ext_cache_inode_drain( [[maybe_unused]] ext_cache_inode_t* cache ) {
}
