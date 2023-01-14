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
#include <common/transaction.h>
#include <common/bfscommon_export.h>

static common_transaction_t* current = NULL;

/**
 * @brief Start a transaction
 *
 * @param write_entry
 * @return int
 */
BFSCOMMON_EXPORT int common_transaction_begin(
  int ( *write_entry )( common_transaction_entry_t* entry )
) {
  if ( ! write_entry ) {
    return EINVAL;
  }
  // handle already ongoing
  if ( current ) {
    return EALREADY;
  }
  // allocate current transaction
  current = malloc( sizeof( *current ) );
  if ( ! current ) {
    return ENOMEM;
  }
  // clear out space
  memset( current, 0, sizeof( *current ) );
  // setup list
  LIST_INIT( &current->block_list );
  // push write entry
  current->write_entry = write_entry;
  // return success
  return EOK;
}

/**
 * @brief Commit running transaction
 *
 * @return int
 */
BFSCOMMON_EXPORT int common_transaction_commit( void ) {
  return ENOSYS;
}

/**
 * @brief Rollback running transaction
 *
 * @return int
 */
BFSCOMMON_EXPORT int common_transaction_rollback( void ) {
  return ENOSYS;
}

/**
 * @brief Push block to transaction
 *
 * @param data
 * @param size
 * @return int
 */
BFSCOMMON_EXPORT int common_transaction_push(
  void* data,
  uint64_t size,
  uint64_t index
) {
  if ( ! data || ! size || ! current ) {
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
  // append block to list
  LIST_INSERT_HEAD( &current->block_list, entry, list );
  // return success
  return EOK;
}
