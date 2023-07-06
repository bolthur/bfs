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

/** @file ext/blockgroup.h */

#include <stdbool.h>
#include <stdint.h>
#include <ext/fs.h>
#include <ext/structure.h>

#ifndef _EXT_BLOCKGROUP_H
#define _EXT_BLOCKGROUP_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined( _BFS_COMPILING )
  int ext_blockgroup_has_superblock( ext_fs_t* fs, uint64_t blockgroup, bool* result );
  int ext_blockgroup_get_by_inode( ext_fs_t* fs, uint64_t inode, uint64_t* result );
  int ext_blockgroup_read( ext_fs_t* fs, uint64_t group, ext_structure_block_group_descriptor_t* value );
  int ext_blockgroup_write( ext_fs_t* fs, uint64_t group, ext_structure_block_group_descriptor_t* value );
#endif

#ifdef __cplusplus
}
#endif

#endif
