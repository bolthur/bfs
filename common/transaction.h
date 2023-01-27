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
#include <stdbool.h>
#include <common/blockdev.h>
#include <thirdparty/queue.h>
#include <thirdparty/tree.h>

#ifndef _COMMON_TRANSACTION_H
#define _COMMON_TRANSACTION_H

#if defined( _BFS_COMPILING )
  #define PARTITION_TREE_DEFINE( name, type, field, cmp, attr ) \
    SPLAY_HEAD( name, type ); \
    SPLAY_PROTOTYPE( name, type, field, cmp ) \
    SPLAY_GENERATE( name, type, field, cmp ) \
    attr void type##_tree_init( struct name* t ) { \
      SPLAY_INIT( t ); \
    } \
    attr int type##_tree_empty( struct name* t ) { \
      return SPLAY_EMPTY( t ); \
    } \
    attr struct type* type##_tree_insert( struct name* t, struct type* e ) { \
      return SPLAY_INSERT( name, t, e ); \
    } \
    attr struct type* type##_tree_remove( struct name* t, struct type* e ) { \
      return SPLAY_REMOVE( name, t, e ); \
    } \
    attr struct type* type##_tree_find( struct name* t, struct type* e ) { \
      return SPLAY_FIND( name, t, e ); \
    } \
    attr struct type* type##_tree_min( struct name* t ) { \
      return SPLAY_MIN( name, t ); \
    } \
    attr struct type* type##_tree_max( struct name* t ) { \
      return SPLAY_MAX( name, t ); \
    } \
    attr struct type* type##_tree_next( struct name* t, struct type* e ) { \
      return SPLAY_NEXT( name, t, e ); \
    } \
    attr void type##_tree_apply( struct name* t, void( *cb )( struct type* ) ) { \
      partition_tree_each( t, type, e, cb( e ) ); \
    } \
    attr void type##_tree_destroy( struct name* t, void( *free_cb )( struct type* ) ) { \
      partition_tree_each_safe( t, type, e, free_cb( type##_tree_remove( t, e ) ) ); \
    }

  #define partition_tree_each( t, type, e, block ) { \
      struct type* e; \
      for ( e = type##_tree_min( t ); e; e = type##_tree_next( t, e ) ) { \
        block; \
      }\
    }

  #define partition_tree_each_safe( t, type, e, block ) { \
      struct type* e; \
      struct type* __tmp; \
      for ( \
        e = type##_tree_min( t ); \
        e && ( __tmp = type##_tree_next( t, e ), e ); \
        e = __tmp \
      ) { \
        block; \
      }\
    }

  /** @brief Transaction entry structure */
  typedef struct common_transaction_entry {
    /** @brief transaction block data */
    void* data;
    /** @brief transaction entry index */
    uint64_t index;
    /** @brief data size */
    uint64_t size;
    /** @brief block count */
    uint64_t block_count;
    /** @brief block device the block is assigned to */
    common_blockdev_t* bdev;
    /** @brief list stuff */
    LIST_ENTRY( common_transaction_entry ) list;
  } common_transaction_entry_t;

  typedef struct common_transaction {
    /** @brief Construct a new list head object */
    LIST_HEAD( transaction_list, common_transaction_entry ) list;
    /**
     * @brief Write transaction entr
     * @param entry entry to write
     * @return int
     */
    int ( *write_entry )( common_transaction_entry_t* entry );
    /** @brief block device transaction is bound to */
    common_blockdev_t* bdev;
    /** @brief node for tree */
    SPLAY_ENTRY( common_transaction ) node;
  } common_transaction_t;

  common_transaction_t* transaction_get( common_blockdev_t* bdev );
  int common_transaction_compare( common_transaction_t* t1, common_transaction_t* t2 );
  int common_transaction_begin( common_blockdev_t* bdev );
  int common_transaction_commit( common_blockdev_t* bdev );
  int common_transaction_rollback( common_blockdev_t* bdev );
  int common_transaction_push( common_blockdev_t* bdev, void* data, uint64_t index, uint64_t size, uint64_t block_count );
  int common_transaction_update( common_blockdev_t* bdev, void* data, uint64_t index, uint64_t size, uint64_t block_count );
  int common_transaction_get( uint64_t index, common_blockdev_t* bdev, common_transaction_entry_t** entry );
  int common_transaction_write( common_transaction_entry_t* entry );
  int common_transaction_running(  common_blockdev_t* bdev, bool* running );
#endif

#endif
