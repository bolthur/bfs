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

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <common/errno.h>
#include <common/lru.h>
#include <common/constant.h>
#include <common/log.h>
#include <common/bfscommon_export.h>

/**
 * @brief lru constructor
 */
INITIALIZER( common_lru_constructor ) {
  atexit( common_lru_destructor );
}

/**
 * @brief lru cache destructor
 */
BFSCOMMON_NO_EXPORT void common_lru_destructor( void ) {
}

/**
 * @brief Setup lru cache with block size
 * @param bdev
 * @param block_size
 * @return common_lru_cache_t*
 */
BFSCOMMON_NO_EXPORT common_lru_cache_t* common_lru_setup(
  common_blockdev_t* bdev,
  const uint64_t block_size
) {
  // handle already set up
  if ( ! block_size ) {
    return nullptr;
  }
  // allocate
  common_lru_cache_t* lru = malloc( sizeof( common_lru_cache_t ) );
  if ( ! lru ) {
    return nullptr;
  }
  // clear out
  memset( lru, 0, sizeof( common_lru_cache_t ) );
  // populate bare minimum
  lru->capacity = LRU_CACHE_CAPACITY;
  lru->node_data_size = block_size;
  lru->bdev = bdev;
  // return success
  return lru;
}

/**
 * Destroy lru cache
 * @param cache
 */
BFSCOMMON_NO_EXPORT void common_lru_destroy( common_lru_cache_t* cache ) {
  common_lru_drain( cache );
  free( cache );
}

/**
 * Drain lru cache
 * @param cache
 */
BFSCOMMON_NO_EXPORT void common_lru_drain( common_lru_cache_t* cache ) {
  if ( ! cache ) {
    return;
  }
  // loop through nodes
  auto node = cache->head;
  while ( node ) {
    // write dirty to persistence
    if ( node->dirty ) {
      common_blockdev_if_bytes_write(
        cache->bdev,
        node->data,
        node->block_number,
        1
      );
    }
    // go to next
    node = node->next;
  }
  // loop through nodes again
  node = cache->head;
  while ( node ) {
    // get next
    auto const next = node->next;
    // detach node
    common_lru_detach( cache, node );
    // free data
    free( node->data );
    free( node );
    // continue with next
    node = next;
  }
  memset( cache->lookup, 0, LRU_CACHE_CAPACITY * sizeof( common_lru_cache_node_t* ) );
}

/**
 * HElper to detach a node
 * @param cache
 * @param node
 */
BFSCOMMON_NO_EXPORT void common_lru_detach(
  common_lru_cache_t* cache,
  common_lru_cache_node_t* node
) {
  if ( ! cache || ! node ) {
    return;
  }
  if ( node->prev ) {
    node->prev->next = node->next;
  }
  if ( node->next ) {
    node->next->prev = node->prev;
  }
  if ( cache->head == node ) {
    cache->head = node->next;
  }
  if ( cache->tail == node ) {
    cache->tail = node->prev;
  }
  node->next = node->prev = nullptr;
}

/**
 * Move node to head
 * @param cache
 * @param node
 */
BFSCOMMON_NO_EXPORT void common_lru_move_to_head(
  common_lru_cache_t* cache,
  common_lru_cache_node_t* node
) {
  if ( ! cache || ! node ) {
    return;
  }
  common_lru_detach( cache, node );
  node->next = cache->head;
  if ( cache->head ) {
    cache->head->prev = node;
  }
  cache->head = node;
  if ( ! cache->tail ) {
    cache->tail = node;
  }
}

/**
 * Get cache node by block number
 * @param cache
 * @param block_number
 * @return
 */
BFSCOMMON_NO_EXPORT common_lru_cache_node_t* common_lru_get(
  common_lru_cache_t* cache,
  const uint64_t block_number
) {
  if ( ! cache ) {
    return nullptr;
  }
  const uint64_t hash = block_number % cache->capacity;
  common_lru_cache_node_t* node = cache->lookup[ hash ];
  // check for match including hash collisions
  while ( node ) {
    if ( node->block_number == block_number ) {
      common_log( "block %"PRIu64" found in cache\r\n", block_number );
      common_lru_move_to_head( cache, node );
      return node;
    }
    node = node->hash_next;
  }
  // cache miss => Handle cache full
  if ( cache->size >= cache->capacity ) {
    common_log( "kicking out oldest block for %"PRIu64"\r\n", block_number );
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
    common_log( "allocating cache for %"PRIu64"\r\n", block_number );
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
  common_log( "read data from %"PRIu64" with result %d\r\n", block_number, result );
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
  return node;
}

/**
 * Write data to cache
 * @param cache
 * @param block_number
 * @param data
 */
BFSCOMMON_NO_EXPORT int common_lru_write(
  common_lru_cache_t* cache,
  const uint64_t block_number,
  const uint8_t* data
) {
  if ( ! cache || ! data ) {
    return EINVAL;
  }
  common_lru_cache_node_t* node = common_lru_get( cache, block_number );
  if ( ! node ) {
    return ENODATA;
  }
  memcpy( node->data, data, ( size_t )cache->node_data_size );
  node->dirty = true;
  return EOK;
}


/**
 * Write data to cache
 * @param cache
 * @param block_number
 * @param data
 */
BFSCOMMON_NO_EXPORT int common_lru_read(
  common_lru_cache_t* cache,
  const uint64_t block_number,
  uint8_t* data
) {
  if ( ! cache || ! data ) {
    return EINVAL;
  }
  common_lru_cache_node_t* node = common_lru_get( cache, block_number );
  if ( ! node ) {
    return ENODATA;
  }
  memcpy( data, node->data, ( size_t )cache->node_data_size );
  return EOK;
}
