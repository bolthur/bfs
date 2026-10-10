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

#include <common/bfscommon_export.h>
#include <ext/cache.h>

/**
 * Helper to create new dentry hash
 * @param parent parent cache entry
 * @param name dentry name
 * @return
 */
BFSCOMMON_NO_EXPORT uint32_t ext_cache_hash_dentry( directory_entry_cache_t* parent, const char* name ) {
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
BFSCOMMON_NO_EXPORT uint32_t ext_cache_hash_inode( ext_fs_t* fs, const uint32_t inode_number ) {
  const uint32_t mount_id = ( uint32_t )( ( uintptr_t )fs );
  const uint32_t hash = ( inode_number ^ mount_id ) * EXT_CACHE_GOLDEN_RATIO;
  return hash & EXT_CACHE_INODE_HASH_MASK;
}
