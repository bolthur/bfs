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

/** @file ext/type.h */

#ifndef EXT_TYPE_H
#define EXT_TYPE_H

#include <stdint.h>
#include <limits.h>
#include <common/mountpoint.h>
#include <ext/structure.h>
#include <ext/fs.h>
#include <ext/cache/directory.h>
#include <ext/cache/inode.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ext_directory ext_directory_t;

/** @brief Ext file definition */
typedef struct ext_file {
  /** @brief Mount point this file is related to */
  common_mountpoint_t *mp;
  /** @brief Flags used while opening the file */
  uint32_t flags;
  /** @brief File size in bytes */
  uint64_t fsize;
  /** @brief Directory containing the file */
  ext_directory_t* dir;
  /** @brief cache entry */
  ext_cache_directory_entry_node_t* cache;
  /** @brief current file position */
  uint64_t fpos;
} ext_file_t;

/** @brief Ext directory structure */
typedef struct ext_directory {
  /** @brief mount point this directory is related to */
  common_mountpoint_t* mp;
  /** @brief Directory data */
  uint8_t* data;
  /** @brief current position */
  uint64_t pos;
  /** @brief directory content size */
  uint64_t entry_size;
  /** @brief cache entry */
  ext_cache_directory_entry_node_t* cache;
  /** @brief current directory entry */
  ext_structure_directory_entry_t* entry;
} ext_directory_t;


#ifdef __cplusplus
}
#endif

#endif
