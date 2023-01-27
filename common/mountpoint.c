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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <thirdparty/queue.h>
#include <common/errno.h>
#include <common/mountpoint.h>
#include <common/lock.h>
#include <common/constant.h>
#include <common/bfscommon_export.h>

static LIST_HEAD( common_mountpoint_list, common_mountpoint ) mountpoint_list;

/**
 * @brief mountpoint management constructor
 */
INITIALIZER( common_mountpoint_constructor ) {
  LIST_INIT( &mountpoint_list );
  atexit( common_mountpoint_destructor );
}

/**
 * @brief mountpoint management destructor
 */
BFSCOMMON_NO_EXPORT void common_mountpoint_destructor( void ) {
}

/**
 * @brief Get mountpoint information by device name
 *
 * @param mountpoint
 * @return common_mountpoint_t*
 */
BFSCOMMON_EXPORT common_mountpoint_t* common_mountpoint_by_mountpoint(
  const char* mountpoint
) {
  size_t mountpoint_length = strlen( mountpoint );
  // loop through list try to find already mounted entry
  common_mountpoint_t* entry;
  LIST_FOREACH( entry, &mountpoint_list, list ) {
    if (
      strlen( entry->name ) == mountpoint_length
      && 0 == strcmp( entry->name, mountpoint )
    ) {
      return entry;
    }
  }
  return NULL;
}

/**
 * @brief Find mountpoint by path
 *
 * @param path
 * @return common_mountpoint_t*
 */
BFSCOMMON_EXPORT common_mountpoint_t* common_mountpoint_find( const char* path ) {
  // loop through list try to find already mounted entry
  common_mountpoint_t* entry;
  LIST_FOREACH( entry, &mountpoint_list, list ) {
    if (
      entry->mounted
      && 0 == strncmp( entry->name, path, strlen( entry->name ) )
    ) {
      return entry;
    }
  }
  return NULL;
}

/**
 * @brief Add mount point
 *
 * @param mountpoint
 * @param fs
 * @param mounted
 * @param lock
 * @return int
 */
BFSCOMMON_EXPORT int common_mountpoint_add(
  const char* mountpoint,
  void* fs,
  bool mounted,
  common_lock_t* lock
) {
  if ( strlen( mountpoint ) > PATH_MAX ) {
    return EINVAL;
  }
  // allocate new entry
  common_mountpoint_t* entry = malloc( sizeof( *entry ) );
  if ( ! entry ) {
    return ENOMEM;
  }
  // copy over name and further properties
  strcpy( entry->name, mountpoint );
  entry->fs = fs;
  entry->mounted = mounted;
  entry->os_lock = lock;
  // insert into list
  LIST_INSERT_HEAD( &mountpoint_list, entry, list );
  // return success
  return EOK;
}

/**
 * @brief Remove mountpoint
 *
 * @param mountpoint
 * @return int
 */
BFSCOMMON_EXPORT int common_mountpoint_remove( const char* mountpoint ) {
  // allocate new entry
  common_mountpoint_t* entry = common_mountpoint_by_mountpoint( mountpoint );
  if ( ! entry ) {
    return ENOENT;
  }
  // remove from list
  LIST_REMOVE( entry, list );
  // free entry
  free( entry );
  // return success
  return EOK;
}

/**
 * @brief Setup lock routines
 *
 * @param mountpoint
 * @param lock
 * @return int
 */
BFSCOMMON_EXPORT int common_mountpoint_setup_lock(
  const char* mountpoint,
  common_lock_t* lock
) {
  // get entry
  common_mountpoint_t* entry = common_mountpoint_by_mountpoint( mountpoint );
  // handle no mount point
  if ( ! entry ) {
    return ENOENT;
  }
  // set os lock property
  entry->os_lock = lock;
  // return success
  return EOK;
}
