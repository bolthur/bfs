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

#include "common/errno.h"

/**
 * Helper to create new dentry hash
 * @param parent parent cache entry
 * @param name dentry name
 * @return
 */
BFSCOMMON_NO_EXPORT uint32_t ext_cache_hash_directory_entry(
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
 * @param fs file system object
 * @return
 */
BFSCOMMON_NO_EXPORT ext_cache_directory_entry_t* ext_cache_directory_entry_setup(
  ext_fs_t* fs
) {
  // allocate
  ext_cache_directory_entry_t* lru = malloc( sizeof( *lru ) );
  if ( ! lru ) {
    return nullptr;
  }
  // clear out
  memset( lru, 0, sizeof( *lru ) );
  // populate bare minimum
  lru->capacity = EXT_CACHE_DENTRY_HASH_SIZE;
  lru->fs = fs;
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
 * @param fs file system object
 * @return
 */
BFSCOMMON_NO_EXPORT ext_cache_inode_t* ext_cache_inode_setup( ext_fs_t* fs ) {
  // allocate
  ext_cache_inode_t* lru = malloc( sizeof( *lru ) );
  if ( ! lru ) {
    return nullptr;
  }
  // clear out
  memset( lru, 0, sizeof( *lru ) );
  // populate bare minimum
  lru->capacity = EXT_CACHE_INODE_HASH_SIZE;
  lru->fs = fs;
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
BFSCOMMON_NO_EXPORT void ext_cache_inode_drain(
  [[maybe_unused]] ext_cache_inode_t* cache
) {
}

/**
 * Helper to get inode from cache
 * @param cache cache to operate on
 * @param value
 * @return
 */
BFSCOMMON_NO_EXPORT ext_cache_inode_node_t* ext_cache_inode_get(
  [[maybe_unused]] ext_cache_inode_t* cache,
  [[maybe_unused]] uint64_t value
) {/*
  if ( ! cache ) {
    return nullptr;
  }
  const uint64_t hash = block_number % cache->capacity;
  common_lru_cache_node_t* node = cache->lookup[ hash ];
  // check for match including hash collisions
  while ( node ) {
    if ( node->block_number == block_number ) {
      common_lru_move_to_head( cache, node );
      return node;
    }
    node = node->hash_next;
  }
  // cache miss => Handle cache full
  if ( cache->size >= cache->capacity ) {
    common_lru_cache_node_t* oldest = cache->tail;
    // handle dirty
    if ( oldest->dirty ) {
      const int result = common_blockdev_if_bytes_write(
        cache->bdev,
        oldest->data,
        block_number,
        1
      );
      if ( result != 0 ) {
        return nullptr;
      }
    }
    // Remove from lookup
    const uint64_t oldest_hash = oldest->block_number % cache->capacity;
    if ( oldest->hash_prev ) {
      oldest->hash_prev->hash_next = oldest->hash_next;
    } else {
      cache->lookup[ oldest_hash ] = oldest->hash_next;
    }
    if ( oldest->hash_next ) {
      oldest->hash_next->hash_prev = oldest->hash_prev;
    }
    // Remove from list and recycle
    common_lru_detach( cache, oldest );
    node = oldest;
  } else {
    node = malloc( sizeof( common_lru_cache_node_t ) );
    if ( ! node ) {
      return nullptr;
    }
    memset( node, 0, sizeof( common_lru_cache_node_t ) );
    node->data = malloc( ( size_t )( sizeof( uint8_t ) * cache->node_data_size ) );
    if ( ! node->data ) {
      free( node );
      return nullptr;
    }
    memset( node->data, 0,  ( size_t )( sizeof( uint8_t ) * cache->node_data_size )  );
    cache->size++;
  }
  // load from persistence
  node->block_number = block_number;
  node->dirty = false;
  const int result = common_blockdev_if_bytes_read(
    cache->bdev,
    node->data,
    block_number,
    1
  );
  if ( result != 0 ) {
    free( node->data );
    free( node );
    return nullptr;
  }
  // add to lookup
  node->prev = node->next = nullptr;
  // add to cache lookup
  node->hash_next = cache->lookup[ hash ];
  node->hash_prev = nullptr;
  if ( node->hash_next ) {
    node->hash_next->hash_prev = node;
  }
  common_lru_move_to_head( cache, node );
  cache->lookup[ hash ] = node;
  // return node
  return node;*/
  return nullptr;
}

/**
 * Write inode
 * @param cache cache to operate on
 * @param value
 * @param in input buffer
 * @return
 */
BFSCOMMON_NO_EXPORT int ext_cache_inode_write(
  ext_cache_inode_t* cache,
  const uint64_t value,
  const uint8_t* in
) {
  if ( ! cache || ! in ) {
    return EINVAL;
  }
  ext_cache_inode_node_t* node = ext_cache_inode_get( cache, value );
  if ( ! node ) {
    return ENODATA;
  }
  node->dirty = true;
  //memcpy( node->data, data, ( size_t )cache->node_data_size );
  return EOK;
}

/**
 * Read inode
 * @param cache cache to operate on
 * @param value
 * @param out output buffer
 * @return
 */
BFSCOMMON_NO_EXPORT int ext_cache_inode_read(
  ext_cache_inode_t* cache,
  const uint64_t value,
  uint8_t* out
) {
  if ( ! cache || ! out ) {
    return EINVAL;
  }
  ext_cache_inode_node_t* node = ext_cache_inode_get( cache, value );
  if ( ! node ) {
    return ENODATA;
  }
  //memcpy( data, node->data, ( size_t )cache->node_data_size );
  return EOK;
}
