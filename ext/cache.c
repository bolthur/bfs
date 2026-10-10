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
#include <common/errno.h>
#include <common/lru.h>
#include <ext/cache.h>
#include <ext/inode.h>
#include <ext/bfsext_export.h>

/**
 * Helper to create new dentry hash
 * @param parent parent cache entry
 * @param name dentry name
 * @return
 */
BFSEXT_NO_EXPORT uint64_t ext_cache_hash_directory_entry(
  ext_cache_directory_entry_node_t* parent,
  const char* name
) {
  uint64_t hash = ( uint64_t )( ( uintptr_t )parent );
  while ( *name ) {
    const uint64_t c = ( uint64_t )( *name++ );
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
BFSEXT_NO_EXPORT uint64_t ext_cache_hash_inode(
  ext_fs_t* fs,
  const uint64_t inode_number
) {
  const uint64_t mount_id = ( uint64_t )( ( uintptr_t )fs );
  const uint64_t hash = ( inode_number ^ mount_id ) * EXT_CACHE_GOLDEN_RATIO;
  return hash & EXT_CACHE_INODE_HASH_MASK;
}

/**
 * Setup directory entry cache
 * @param fs file system object
 * @return
 */
BFSEXT_NO_EXPORT ext_cache_directory_entry_t* ext_cache_directory_entry_setup(
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
 */
BFSEXT_NO_EXPORT void ext_cache_directory_entry_destroy(
  ext_cache_directory_entry_t* cache
) {
  ext_cache_directory_entry_drain( cache );
  free( cache );
}

/**
 * Detach inode
 * @param cache cache to operate on
 * @param node node to detach
 */
BFSEXT_NO_EXPORT void ext_cache_directory_entry_detach(
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
BFSEXT_NO_EXPORT void ext_cache_directory_entry_move_to_head(
  ext_cache_directory_entry_t* cache,
  ext_cache_directory_entry_node_t* node
) {
  if ( ! cache || ! node ) {
    return;
  }
  ext_cache_directory_entry_detach( cache, node );
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
 * Drain directory cache
 * @param cache cache to drain
 */
BFSEXT_NO_EXPORT void ext_cache_directory_entry_drain(
  ext_cache_directory_entry_t* cache
) {
  auto node = cache->head;
  while ( node ) {
    // cache next
    auto const next = node->lru_next;
    // handle still referenced
    if ( node->reference_count ) {
      node = next;
      continue;
    }
    // dereference inode in case it is set
    if ( node->inode ) {
      ext_cache_inode_put( cache->fs->inode, node->inode_number );
    }
    // detach node
    ext_cache_directory_entry_detach( cache, node );
    ext_cache_directory_entry_hash_remove( cache, node );
    // free data
    free( node );
    // continue with next
    node = next;
  }
}

/**
 * Get directory entry cache
 * @param cache cache to operate on
 * @param parent parent node
 * @param name folder name
 * @return
 */
BFSEXT_NO_EXPORT ext_cache_directory_entry_node_t* ext_cache_directory_entry_get(
  ext_cache_directory_entry_t* cache,
  ext_cache_directory_entry_node_t* parent,
  const char* name
) {
  if ( ! cache ) {
    return nullptr;
  }
  // lookup via hash
  ext_cache_directory_entry_node_t* node = ext_cache_directory_entry_lookup( cache, parent, name );
  // handle found
  if ( node ) {
    // increment count to prevent drop
    node->reference_count++;
    // return found node
    return node;
  }
  // allocate or evict a node
  node = ext_cache_directory_entry_allocate_or_evict( cache );
  // handle nothing
  if ( ! node ) {
    return nullptr;
  }
  // populate parent
  node->parent = parent;
  // special case for roor
  if ( ! node->parent ) {
    // set root inode number
    node->inode_number = EXT_INODE_EXT2_ROOT_INO;
    // load inode from cache
    node->inode = ext_cache_inode_get( cache->fs->inode, node->inode_number );
    if ( ! node->inode ) {
      free( node );
      return nullptr;
    }
  } else {
    // handle no directory
    if (  ! ( parent->inode->inode.i_mode & EXT_INODE_EXT2_S_IFDIR ) ) {
      free( node );
      return nullptr;
    }
    // allocate temporary space for folder data
    uint8_t* data = malloc( parent->inode->inode.i_size );
    if ( ! data ) {
      free( node );
      return nullptr;
    }
    // load folder data into temporary buffer
    const int result = ext_inode_read_data(
      cache->fs,
      &parent->inode->inode,
      0,
      parent->inode->inode.i_size,
      data
    );
    // handle error
    if ( EOK != result ) {
      free( data );
      free( node );
      return nullptr;
    }
    // loop through directory
    uint64_t offset = 0;
    ext_structure_directory_entry_t* found = nullptr;
    while ( offset < parent->inode->inode.i_size ) {
      auto const entry = ( ext_structure_directory_entry_t* )( data + offset );
      // handle end reached
      if ( entry->rec_len == 0 ) {
        break;
      }
      // handle valid inode
      if ( entry->inode != 0 ) {
        // when name length and name matches use it
        if (
          entry->name_len == strlen( name )
          && 0 == strncmp( entry->name, name, entry->name_len )
        ) {
          found = entry;
          break;
        }
      }
      // get to next entry
      offset += entry->rec_len;
    }
    // handle not found
    if ( ! found ) {
      free( data );
      free( node );
      return nullptr;
    }
    // populate node
    node->inode_number = found->inode;
    node->inode = ext_cache_inode_get( cache->fs->inode, node->inode_number );
    // unset found again
    found = nullptr;
    // free temporary data again
    free( data );
  }
  // copy over name and initialize reference count
  strcpy( node->name, name );
  node->reference_count = 1;
  // insert node
  ext_cache_directory_entry_insert( cache, node );
  // return node
  return node;
}

/**
 * Inode cache lookup helper
 * @param cache cache to operate on
 * @param parent parent directory
 * @param path
 * @return
 */
BFSEXT_NO_EXPORT ext_cache_directory_entry_node_t* ext_cache_directory_entry_lookup(
  ext_cache_directory_entry_t* cache,
  ext_cache_directory_entry_node_t* parent,
  const char* path
) {
  if ( ! cache ) {
    return nullptr;
  }
  const uint64_t hash = ext_cache_hash_directory_entry( parent, path );
  ext_cache_directory_entry_node_t* node = cache->lookup[ hash ];
  // check for match including hash collisions
  while ( node ) {
    if (
      node->parent == parent
      && strlen( node->name ) == strlen( path )
      && 0 == strcmp( node->name, path )
    ) {
      ext_cache_directory_entry_move_to_head( cache, node );
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
BFSEXT_NO_EXPORT void ext_cache_directory_entry_insert(
  ext_cache_directory_entry_t* cache,
  ext_cache_directory_entry_node_t* node
) {
  const uint64_t hash = ext_cache_hash_directory_entry( node->parent, node->name );
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
  ext_cache_directory_entry_move_to_head( cache, node );
}

/**
 * Helper to remove inode from hash lookup
 * @param cache
 * @param node
 */
BFSEXT_NO_EXPORT void ext_cache_directory_entry_hash_remove(
  ext_cache_directory_entry_t* cache,
  const ext_cache_directory_entry_node_t* node
) {
  const uint64_t hash = ext_cache_hash_directory_entry( node->parent, node->name );
  auto current = cache->lookup[ hash ];
  while ( current ) {
    if (
      strlen( current->name ) == strlen( node->name ) &&
      0 == strcmp( current->name, node->name )
    ) {
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
BFSEXT_NO_EXPORT ext_cache_directory_entry_node_t* ext_cache_directory_entry_allocate_or_evict(
  ext_cache_directory_entry_t* cache
) {
  // handle space left
  if ( cache->size < cache->capacity ) {
    // allocate new node
    ext_cache_directory_entry_node_t* node = malloc( sizeof( *node ) );
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
  ext_cache_directory_entry_node_t* oldest = cache->tail;
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
  // remove from hash table
  ext_cache_directory_entry_hash_remove( cache, oldest );
  // dereference inode
  ext_cache_inode_put( cache->fs->inode, oldest->inode_number );
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
 * Dereference a node up to it's parent
 * @param node
 */
BFSEXT_NO_EXPORT void ext_cache_directory_entry_put( ext_cache_directory_entry_node_t* node ) {
  while ( node ) {
    node->reference_count--;
    node = node->parent;
  }
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
BFSEXT_NO_EXPORT void ext_cache_inode_put( ext_cache_inode_t* cache, uint64_t inode_number ) {
  // lookup via hash
  ext_cache_inode_node_t* node = ext_cache_inode_lookup( cache, inode_number );
  // handle found
  if ( node ) {
    // increment count to prevent drop
    node->reference_count--;
  }
}
