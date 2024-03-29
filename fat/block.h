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

/** @file fat/block.h */

#include <stdbool.h>
#include <stdint.h>
#include <fat/type.h>

#ifndef _FAT_BLOCK_H
#define _FAT_BLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined( _BFS_COMPILING )
  int fat_block_load( fat_file_t* file, uint64_t size );
  int fat_block_unload( fat_file_t* file );
  int fat_block_load_directory( fat_directory_t* dir, bool reload );
  int fat_block_unload_directory( fat_directory_t* dir );
  int fat_block_write( fat_file_t* file, uint64_t size );
  int fat_block_write_directory( fat_directory_t* dir );
#endif

#ifdef __cplusplus
}
#endif

#endif
