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

/** @file ext/directory.h */

#include <stdint.h>
#include <ext/fs.h>
#include <ext/structure.h>
#include <ext/type.h>

#ifndef _EXT_DIRECTORY_H
#define _EXT_DIRECTORY_H

#ifdef __cplusplus
extern "C" {
#endif

int ext_directory_remove( const char* path );
int ext_directory_move( const char* old_path, const char* new_path );
int ext_directory_make( const char* path );
int ext_directory_open( ext_directory_t* dir, const char* path );
int ext_directory_close( ext_directory_t* dir );
int ext_directory_next_entry( ext_directory_t* dir );
int ext_directory_rewind( ext_directory_t* dir );
int ext_directory_entry_by_name( ext_directory_t* dir, const char* name );

#if defined( _BFS_COMPILING )
  int ext_directory_load( ext_fs_t* fs, ext_directory_t* dir, ext_structure_inode_t* inode );
  int ext_directory_entry_size( uint64_t name_length, uint64_t* value );
#endif

#ifdef __cplusplus
}
#endif

#endif
