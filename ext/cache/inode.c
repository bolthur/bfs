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
#include <inttypes.h>
#include <common/errno.h>
#include <common/lru.h>
#include <common/log.h>
#include <ext/cache/inode.h>
#include <ext/inode.h>
#include <ext/bfsext_export.h>

/**
 * Create hash of inode
 * @param fs file system object
 * @param inode_number inode number
 * @return
 */
BFSEXT_NO_EXPORT uint64_t ext_cache_hash_inode(
  ext_fs_t* fs,
  const uint64_t inode_number
) {
  const uint64_t mount_id = ( uint64_t )( ( uintptr_t )fs );
  const uint64_t hash = ( inode_number ^ mount_id ) * EXT_CACHE_GOLDEN_RATIO;
  return hash & EXT_CACHE_INODE_HASH_MASK;
}

/**
 * Setup inode cache
 * @param fs file system object
 * @return
 */
BFSEXT_NO_EXPORT ext_cache_inode_t* ext_cache_inode_setup( ext_fs_t* fs ) {
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
 */
BFSEXT_NO_EXPORT void ext_cache_inode_destroy(
  ext_cache_inode_t* cache
) {
  ext_cache_inode_drain( cache );
  free( cache );
}

/**
 * Detach inode
 * @param cache cache to operate on
 * @param node node to detach
 */
BFSEXT_NO_EXPORT void ext_cache_inode_detach(
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
BFSEXT_NO_EXPORT void ext_cache_inode_move_to_head(
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
 */
BFSEXT_NO_EXPORT void ext_cache_inode_drain( ext_cache_inode_t* cache ) {
  auto node = cache->head;
  while ( node ) {
    // cache next
    auto const next = node->lru_next;
    // handle still referenced
    if ( node->reference_count ) {
      node = next;
      continue;
    }
    // flush inode
    ext_cache_inode_flush( cache, node );
    // detach node
    ext_cache_inode_detach( cache, node );
    ext_cache_inode_hash_remove( cache, node );
    // free data
    free( node );
    // continue with next
    node = next;
  }
}

/**
 * Helper to get inode from cache
 * @param cache cache to operate on
 * @param inode_number
 * @return
 */
BFSEXT_NO_EXPORT ext_cache_inode_node_t* ext_cache_inode_get(
  ext_cache_inode_t* cache,
  const uint64_t inode_number
) {
  if ( ! cache ) {
    return nullptr;
  }
  // lookup via hash
  ext_cache_inode_node_t* node = ext_cache_inode_lookup( cache, inode_number );
  // handle found
  if ( node ) {
    // increment count to prevent drop
    node->reference_count++;
    // return found node
    return node;
  }
  // allocate or evict a node
  node = ext_cache_inode_allocate_or_evict( cache );
  // handle nothing
  if ( ! node ) {
    return nullptr;
  }
  // evaluate where inode is located
  uint64_t block;
  uint64_t offset;
  const int result = ext_inode_calculate_block_number(
    cache->fs, inode_number, &block, &offset );
  if ( EOK != result ) {
    free( node );
    return nullptr;
  }
  common_log( "Loading inode %"PRIu64" from block %"PRIu64" with offset %"PRIu64"\r\n",
    inode_number, block, offset )
  // load cache via lru
  common_lru_cache_node_t* lru = common_lru_get( cache->fs->bdev->lru, block );
  if ( ! lru ) {
    free( node );
    return nullptr;
  }
  // get inode itself
  auto const inode = ( ext_structure_inode_t* )( lru->data + offset );
  // populate inode structure
  node->inode_number = inode_number;
  node->reference_count = 1;
  node->dirty = false;
  // copy inode data
  memcpy( &node->inode, inode, sizeof( ext_structure_inode_t ) );
  common_log( "node->inode.i_mode = %#"PRIx16"\r\n", node->inode.i_mode );
  // insert node
  ext_cache_inode_insert( cache, node );
  // return node
  return node;
}

/**
 * Write inode
 * @param cache cache to operate on
 * @param inode_number
 * @param in input buffer
 * @return
 */
BFSEXT_NO_EXPORT int ext_cache_inode_write(
  ext_cache_inode_t* cache,
  const uint64_t inode_number,
  const uint8_t* in
) {
  if ( ! cache || ! in ) {
    return EINVAL;
  }
  // get cached inode
  ext_cache_inode_node_t* node = ext_cache_inode_get( cache, inode_number );
  if ( ! node ) {
    return ENODATA;
  }
  // mark as dirty
  node->dirty = true;
  // copy content
  memcpy( &node->inode, in, sizeof( ext_structure_inode_t ) );
  // decrement inode again
  ext_cache_inode_put( cache, inode_number );
  // return success
  return EOK;
}

/**
 * Read inode
 * @param cache cache to operate on
 * @param inode_number
 * @param out output buffer
 * @return
 */
BFSEXT_NO_EXPORT int ext_cache_inode_read(
  ext_cache_inode_t* cache,
  const uint64_t inode_number,
  uint8_t* out
) {
  if ( ! cache || ! out ) {
    return EINVAL;
  }
  // get cached inode
  ext_cache_inode_node_t* node = ext_cache_inode_get( cache, inode_number );
  if ( ! node ) {
    return ENODATA;
  }
  // copy content
  memcpy( out, &node->inode, sizeof( ext_structure_inode_t ) );
  // decrement inode again
  ext_cache_inode_put( cache, inode_number );
  // return success
  return EOK;
}

/**
 * Inode cache lookup helper
 * @param cache cache to operate on
 * @param inode_number inode number
 * @return
 */
BFSEXT_NO_EXPORT ext_cache_inode_node_t* ext_cache_inode_lookup(
  ext_cache_inode_t* cache,
  const uint64_t inode_number
) {
  if ( ! cache ) {
    return nullptr;
  }
  const uint64_t hash = ext_cache_hash_inode( cache->fs, inode_number );
  ext_cache_inode_node_t* node = cache->lookup[ hash ];
  // check for match including hash collisions
  while ( node ) {
    if ( node->inode_number == inode_number ) {
      ext_cache_inode_move_to_head( cache, node );
      return node;
    }
    node = node->hash_next;
  }
  return nullptr;
}

/**
 * Insert inode into cache
 * @param cache cache to operate on
 * @param node node to insert
 */
BFSEXT_NO_EXPORT void ext_cache_inode_insert(
  ext_cache_inode_t* cache,
  ext_cache_inode_node_t* node
) {
  const uint64_t hash = ext_cache_hash_inode( cache->fs, node->inode_number );
  // set hash next to lookup
  node->hash_next = cache->lookup[ hash ];
  node->hash_prev = nullptr;
  // set previous of next
  if ( node->hash_next ) {
    node->hash_next->hash_prev = node;
  }
  // overwrite lookup
  cache->lookup[ hash ] = node;
  // enqueue in lru list
  ext_cache_inode_move_to_head( cache, node );
}

/**
 * Helper to remove inode from hash lookup
 * @param cache
 * @param node
 */
BFSEXT_NO_EXPORT void ext_cache_inode_hash_remove(
  ext_cache_inode_t* cache,
  const ext_cache_inode_node_t* node
) {
  const uint64_t hash = ext_cache_hash_inode( cache->fs, node->inode_number );
  auto current = cache->lookup[ hash ];
  while ( current ) {
    if ( current->inode_number == node->inode_number ) {
      if ( current->hash_prev ) {
        current->hash_prev->hash_next = current->hash_next;
      } else {
        cache->lookup[ hash ] = current->hash_next;
      }
      if ( current->hash_next ) {
        current->hash_next->hash_prev = current->hash_prev;
      }
    }
    current = current->hash_next;
  }
}

/**
 * Allocate new node or evict existing one
 * @param cache cache to operate on
 * @return
 */
BFSEXT_NO_EXPORT ext_cache_inode_node_t* ext_cache_inode_allocate_or_evict(
  ext_cache_inode_t* cache
) {
  // handle space left
  if ( cache->size < cache->capacity ) {
    // allocate new node
    ext_cache_inode_node_t* node = malloc( sizeof( *node ) );
    // handle error
    if ( ! node ) {
      return nullptr;
    }
    // clear out
    memset( node, 0, sizeof( *node ) );
    // increment cache size
    cache->size++;
    // return new node
    return node;
  }
  // Handle cache full
  ext_cache_inode_node_t* oldest = cache->tail;
  while ( oldest ) {
    // found one with no reference, so break
    if ( ! oldest->reference_count ) {
      break;
    }
    // go to next
    oldest = oldest->lru_next;
  }
  // handle nothing found
  if ( ! oldest ) {
    return nullptr;
  }
  // write to disk if dirty
  if ( oldest->dirty ) {
    ext_cache_inode_flush( cache, oldest );
  }
  // remove from hash table
  ext_cache_inode_hash_remove( cache, oldest );
  // remove from list
  if ( oldest->lru_prev ) {
    oldest->lru_prev->lru_next = oldest->lru_next;
  }
  if ( oldest->lru_next ) {
    oldest->lru_next->lru_prev = oldest->lru_prev;
  }
  if ( cache->head == oldest ) {
    cache->head = oldest->lru_next;
  }
  if ( cache->tail == oldest ) {
    cache->tail = oldest->lru_prev;
  }
  // clear out data
  memset( oldest, 0, sizeof( *oldest ) );
  // finally return oldest
  return oldest;
}

/**
 * flush inode to persistence
 * @param cache cache to operate on
 * @param node node to flush
 */
BFSEXT_NO_EXPORT void ext_cache_inode_flush( ext_cache_inode_t* cache, ext_cache_inode_node_t* node ) {
  if ( ! cache || ! node->dirty ) {
    return;
  }
  // get block and offset
  uint64_t block;
  uint64_t offset;
  const int result = ext_inode_calculate_block_number( cache->fs, node->inode_number, &block, &offset );
  if ( EOK != result ) {
    return;
  }
  // get cache node from lru
  common_lru_cache_node_t* lru = common_lru_get( cache->fs->bdev->lru, block );
  if ( ! lru ) {
    return;
  }
  // modify from memory
  auto const inode = ( ext_structure_inode_t* )( lru->data + offset );
  memcpy( inode, &node->inode, sizeof( *inode ) );
  // mark lru node as dirty
  lru->dirty = true;
  // mark inode as sane
  node->dirty = false;
}

/**
 * Put inode from cache
 * @param cache cache to operate on
 * @param inode_number inode number
 */
BFSEXT_NO_EXPORT void ext_cache_inode_put(
  ext_cache_inode_t* cache,
  const uint64_t inode_number
) {
  // lookup via hash
  ext_cache_inode_node_t* node = ext_cache_inode_lookup( cache, inode_number );
  // handle found
  if ( node ) {
    // increment count to prevent drop
    node->reference_count--;
  }
}
