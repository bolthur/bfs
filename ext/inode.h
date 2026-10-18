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

/** @file ext/inode.h */

#ifndef EXT_INODE_H
#define EXT_INODE_H

#include <stdint.h>
#include <ext/fs.h>
#include <ext/structure.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief inode allocation enum */
typedef enum {
  /** @brief no allocation */
  INODE_NO_ALLOC = 0,
  /** @brief allocate */
  INODE_ALLOC = 1
} inode_allocate_t;

#if defined( _BFS_COMPILING )
  int ext_inode_get_local_inode( ext_fs_t* fs, uint64_t inode, uint64_t* result );
  int ext_inode_read_inode( ext_fs_t* fs, uint64_t number, ext_structure_inode_t* inode );
  int ext_inode_write_inode( ext_fs_t* fs, uint64_t number, ext_structure_inode_t* inode );
  int ext_inode_read_data( ext_fs_t* fs, ext_structure_inode_t* inode, uint64_t start, uint64_t length, uint8_t* buffer );
  int ext_inode_write_data( ext_fs_t* fs, ext_structure_inode_t* inode, uint64_t start, uint64_t length, uint8_t* buffer );
  int ext_inode_read_block( ext_fs_t* fs, ext_structure_inode_t* inode, uint64_t block_no, uint8_t* buffer, uint64_t count );
  int ext_inode_write_block( ext_fs_t* fs, ext_structure_inode_t* inode, uint64_t block_no, uint8_t* buffer, uint64_t count );
  int ext_inode_get_block_offset( ext_fs_t* fs, ext_structure_inode_t* inode, uint64_t block_no, uint64_t* offset, inode_allocate_t allocate );
  int ext_inode_allocate( ext_fs_t* fs, ext_structure_inode_t* inode, uint64_t* inode_number );
  int ext_inode_deallocate_block_recursive( ext_fs_t* fs, uint64_t table_block, uint64_t level );
  int ext_inode_deallocate( ext_fs_t* fs, ext_structure_inode_t* inode, uint64_t number );
  int ext_inode_calculate_block_number( ext_fs_t* fs, uint64_t inode_number, uint64_t* out_block_number, uint64_t* out_block_offset );
#endif

#ifdef __cplusplus
}
#endif

#endif
