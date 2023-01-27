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

/** @file common/mountpoint.h */

#include <stdbool.h>
#include <limits.h>
#include <thirdparty/queue.h>
#include <common/lock.h>

#ifndef _COMMON_MOUNTPOINT_H
#define _COMMON_MOUNTPOINT_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Macro to execute os lock method if set */
#define COMMON_MP_LOCK(m) \
  if ((m)->os_lock) { \
    (m)->os_lock->lock(); \
  }

/** @brief Macro to execute os unlock method if set */
#define COMMON_MP_UNLOCK(m) \
  if ((m)->os_lock) { \
    (m)->os_lock->unlock(); \
  }

/** @brief Common mount point structure */
typedef struct common_mountpoint {
  /** @brief Flag indicating whether mountpoint is actually mounted */
  bool mounted;
  /** @brief Name of the mountpoint */
  char name[ PATH_MAX ];
  /** @brief Pointer to file system structure */
  void* fs;
  /** @brief Operating system lock interface */
  common_lock_t* os_lock;
  /** @brief list management entry */
  LIST_ENTRY( common_mountpoint ) list;
} common_mountpoint_t;

#if defined( _BFS_COMPILING )
  void common_mountpoint_destructor( void );
#endif

int common_mountpoint_setup_lock( const char* mountpoint, common_lock_t* lock );
common_mountpoint_t* common_mountpoint_by_mountpoint( const char* mountpoint );
common_mountpoint_t* common_mountpoint_find( const char* path );
int common_mountpoint_add( const char* mountpoint, void* fs, bool mounted, common_lock_t* lock );
int common_mountpoint_remove( const char* mountpoint );

#ifdef __cplusplus
}
#endif

#endif
