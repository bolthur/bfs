// Copyright (C) 2022 - 2023 bolthur project.
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

// IWYU pragma: no_include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <common/errno.h>
#include <common/blockdev.h>
#include <common/transaction.h>
#include <common/bfscommon_export.h>
#include <common/constant.h>
#include <thirdparty/queue.h>
#include <bfsconfig.h>

// define tree
PARTITION_TREE_DEFINE(
  transaction_tree,
  common_transaction,
  node,
  common_transaction_compare,
  __unused static inline
)

// create static tree
static struct transaction_tree management_tree;

/**
 * @brief Helepr to lookup a transaction
 *
 * @param bdev
 * @return common_transaction_t*
 */
BFSCOMMON_NO_EXPORT common_transaction_t* transaction_get(
  common_blockdev_t* bdev
) {
  // allocate node
  common_transaction_t* node = malloc( sizeof( *node ) );
  // handle error
  if ( ! node ) {
    return NULL;
  }
  // clear out node
  memset( node, 0, sizeof( *node ) );
  // set bdev
  node->bdev = bdev;
  // lookup for node
  common_transaction_t* found = common_transaction_tree_find(
    &management_tree,
    node
  );
  // free node
  free( node );
  // return found
  return found;
}


/**
 * @brief Common transaction compare
 *
 * @param a
 * @param b
 * @return int
 */
BFSCOMMON_NO_EXPORT int common_transaction_compare(
  common_transaction_t* t1,
  common_transaction_t* t2
) {
  return t1->bdev == t2->bdev ? 0 : 1;
}

/**
 * @brief Start a transaction
 *
 * @param bdev
 * @return int
 */
BFSCOMMON_NO_EXPORT int common_transaction_begin( common_blockdev_t* bdev ) {
  // disable transaction
  #if 1 != CONFIG_USE_TRANSACTION
    return EOK;
  #endif
  // handle invalid
  if ( ! bdev ) {
    return EINVAL;
  }
  // handle already ongoing
  if ( transaction_get( bdev ) ) {
    return EALREADY;
  }
  // allocate current transaction
  common_transaction_t* current = malloc( sizeof( *current ) );
  if ( ! current ) {
    return ENOMEM;
  }
  // clear out space
  memset( current, 0, sizeof( *current ) );
  // setup list
  LIST_INIT( &current->list );
  // push write entry
  current->write_entry = common_transaction_write;
  current->bdev = bdev;
  // insert into tree
  if ( common_transaction_tree_insert( &management_tree, current ) ) {
    free( current );
    return EIO;
  }
  // return success
  return EOK;
}

/**
 * @brief Commit running transaction
 *
 * @param bdev
 * @return int
 */
BFSCOMMON_NO_EXPORT int common_transaction_commit( common_blockdev_t* bdev ) {
  // disable transaction
  #if 1 != CONFIG_USE_TRANSACTION
    return EOK;
  #endif
  // handle invalid
  if ( ! bdev ) {
    return EINVAL;
  }
  common_transaction_t* t = transaction_get( bdev );
  // handle no transaction ongoing
  if ( ! t ) {
    return EINVAL;
  }
  common_transaction_entry_t* e;
  // loop through list and write
  LIST_FOREACH( e, &t->list, list ) {
    // try to write
    int result = t->write_entry( e );
    // handle error
    if ( EOK != result ) {
      return result;
    }
  }
  // cleanup entries
  while ( LIST_FIRST( &t->list ) ) {
    // get element
    common_transaction_entry_t* remove = LIST_FIRST( &t->list );
    // remove element
    LIST_REMOVE( remove, list );
    // free data
    if ( remove->data ) {
      free( remove->data );
    }
    free( remove );
  }
  // remove transaction from tree
  t = common_transaction_tree_remove( &management_tree, t );
  // free transaction
  if ( t ) {
    free( t );
  }
  // return success
  return EOK;
}

/**
 * @brief Rollback running transaction
 *
 * @param bdev
 * @return int
 */
BFSCOMMON_NO_EXPORT int common_transaction_rollback( common_blockdev_t* bdev ) {
  // disable transaction
  #if 1 != CONFIG_USE_TRANSACTION
    return EOK;
  #endif
  // handle invalid
  if ( ! bdev ) {
    return EINVAL;
  }
  common_transaction_t* t = transaction_get( bdev );
  // handle no transaction ongoing
  if ( ! t ) {
    return EINVAL;
  }
  // cleanup entries
  while ( LIST_FIRST( &t->list ) ) {
    // get element
    common_transaction_entry_t* remove = LIST_FIRST( &t->list );
    // remove element
    LIST_REMOVE( remove, list );
    // free data
    if ( remove->data ) {
      free( remove->data );
    }
    free( remove );
  }
  // remove transaction from tree
  t = common_transaction_tree_remove( &management_tree, t );
  // free transaction
  if ( t ) {
    free( t );
  }
  // return success
  return EOK;
}

/**
 * @brief Push block to transaction
 *
 * @param bdev
 * @param data
 * @param index
 * @param size
 * @param block_count
 * @return int
 */
BFSCOMMON_NO_EXPORT int common_transaction_push(
  common_blockdev_t* bdev,
  void* data,
  uint64_t index,
  uint64_t size,
  uint64_t block_count
) {
  // disable transaction
  #if 1 != CONFIG_USE_TRANSACTION
    return EOK;
  #endif
  // validate
  if ( ! data || ! size || ! bdev ) {
    return EINVAL;
  }
  common_transaction_t* t = transaction_get( bdev );
  // handle no transaction ongoing
  if ( ! t ) {
    return EINVAL;
  }
  // allocate entry
  common_transaction_entry_t* entry = malloc( sizeof( *entry ) );
  if ( ! entry ) {
    return ENOMEM;
  }
  // allocate data
  entry->data = malloc( size );
  if ( ! entry->data ) {
    free( entry );
    return ENOMEM;
  }
  // copy over data
  memcpy( entry->data, data, size );
  entry->index = index;
  entry->size = size;
  entry->bdev = bdev;
  entry->block_count = block_count;
  // append block to list
  LIST_INSERT_HEAD( &t->list, entry, list );
  // return success
  return EOK;
}

/**
 * @brief Update block in transaction
 *
 * @param bdev
 * @param data
 * @param index
 * @param size
 * @param block_count
 * @return int
 */
BFSCOMMON_NO_EXPORT int common_transaction_update(
  common_blockdev_t* bdev,
  void* data,
  uint64_t index,
  uint64_t size,
  uint64_t block_count
) {
  // disable transaction
  #if 1 != CONFIG_USE_TRANSACTION
    return EOK;
  #endif
  // validate
  if ( ! data || ! size || ! bdev ) {
    return EINVAL;
  }
  // get possible block
  common_transaction_entry_t* entry;
  int result = common_transaction_get( index, bdev, &entry );
  if ( EOK != result ) {
    return result;
  }
  // insert if not existing
  if ( ! entry ) {
    return common_transaction_push( bdev, data, index, size, block_count );
  }
  // validate data
  if (
    entry->bdev != bdev
    || entry->index != index
    || entry->size != size
    || entry->block_count != block_count
  ) {
    return EINVAL;
  }
  // overwrite data
  memcpy( entry->data, data, size );
  // return success
  return EOK;
}

/**
 * @brief Method to get transaction entry by index
 *
 * @param index
 * @param bdev
 * @param entry
 * @return int
 */
BFSCOMMON_NO_EXPORT int common_transaction_get(
  uint64_t index,
  common_blockdev_t* bdev,
  common_transaction_entry_t** entry
) {
  // disable transaction
  #if 1 != CONFIG_USE_TRANSACTION
    *entry = NULL;
    return EOK;
  #endif
  // validate parameter
  if ( ! entry || ! bdev ) {
    return EINVAL;
  }
  // setup entry
  *entry = NULL;
  // get transaction
  common_transaction_t* t = transaction_get( bdev );
  // handle no transaction ongoing
  if ( ! t ) {
    return EOK;
  }
  common_transaction_entry_t* e;
  // loop through list and return
  LIST_FOREACH( e, &t->list, list ) {
    if ( e->index == index && e->bdev == bdev ) {
      // set entry
      *entry = e;
      return EOK;
    }
  }
  // return success
  return EOK;
}

/**
 * @brief Method to write specific transaction entry
 *
 * @param entry
 * @return int
 */
BFSCOMMON_NO_EXPORT int common_transaction_write(
  common_transaction_entry_t* entry
) {
  // disable transaction
  #if 1 != CONFIG_USE_TRANSACTION
    return EOK;
  #endif
  common_blockdev_if_lock( entry->bdev );
  int result = entry->bdev->bdif->write(
    entry->bdev,
    entry->data,
    entry->index,
    entry->block_count
  );
  entry->bdev->bdif->read_counter++;
  common_blockdev_if_unlock( entry->bdev );
  return result;
}

/**
 * @brief Helper to get whether transaction is running
 *
 * @param bdev
 * @param running
 * @return int
 */
BFSCOMMON_NO_EXPORT int common_transaction_running(
  common_blockdev_t* bdev,
  bool* running
) {
  if ( ! running || ! bdev ) {
    return EINVAL;
  }
  // get transaction
  common_transaction_t* t = transaction_get( bdev );
  *running = t;
  return EOK;
}
