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

/**
 * @file common/transaction.h
 */

#include <stdint.h>
#include <common/sys/queue.h>

#ifndef _COMMON_TRANSACTION_H
#define _COMMON_TRANSACTION_H

/** @brief Transaction entry structure */
typedef struct common_transaction_entry {
  /** @brief transaction block data */
  void* data;
  /** @brief data size */
  uint64_t size;
  /** @brief transaction entry index */
  uint64_t index;
  /** @brief list stuff */
  LIST_ENTRY( common_transaction_entry ) list;
} common_transaction_entry_t;

/** @brief transaction structure */
typedef struct {
  /** @brief Construct a new list head object */
  LIST_HEAD( transaction_list, common_transaction_entry ) block_list;
  /**
   * @brief Write transaction entr
   * @param entry entry to write
   * @return int
   */
  int ( *write_entry )( common_transaction_entry_t* entry );
} common_transaction_t;

int common_transaction_begin( int ( *write_entry )( common_transaction_entry_t* entry ) );
int common_transaction_commit( void );
int common_transaction_rollback( void );
int common_transaction_push( void* data, uint64_t size, uint64_t index );

#endif
