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

/** @file ext/block.h */

#include <stdint.h>
#include <ext/fs.h>
#include <ext/structure.h>

#ifndef _EXT_BLOCK_H
#define _EXT_BLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined( _BFS_COMPILING )
  int ext_block_read_bitmap( ext_fs_t* fs, ext_structure_block_group_descriptor_t* descriptor, uint32_t* value );
  int ext_block_write_bitmap( ext_fs_t* fs, ext_structure_block_group_descriptor_t* descriptor, uint32_t* value );
  int ext_block_allocate( ext_fs_t* fs, uint64_t* value );
  int ext_block_deallocate( ext_fs_t* fs, uint64_t value );
#endif

#ifdef __cplusplus
}
#endif

#endif
