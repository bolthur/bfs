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

/** @file fat/directory.h */

#include <stdint.h>
#include <stdbool.h>
#include <fat/type.h>
#include <fat/structure.h>

#ifndef _FAT_DIRECTORY_H
#define _FAT_DIRECTORY_H

#ifdef __cplusplus
extern "C" {
#endif

int fat_directory_remove( const char* path );
int fat_directory_move( const char* old_path, const char* new_path );
int fat_directory_make( const char* path );
int fat_directory_open( fat_directory_t* dir, const char* path );
int fat_directory_close( fat_directory_t* dir );
int fat_directory_next_entry( fat_directory_t* dir );
int fat_directory_rewind( fat_directory_t* dir );
int fat_directory_entry_by_name( fat_directory_t* dir, const char* path );

#if defined( _BFS_COMPILING )
  int fat_directory_size( fat_directory_t* dir, uint64_t* size );
  int fat_directory_entry_is_valid( fat_structure_directory_entry_t* entry, bool* is_valid );
  int fat_directory_entry_is_free( fat_structure_directory_entry_t* entry, bool* is_free );
  int fat_directory_entry_is_dot( fat_structure_directory_entry_t* entry, bool* is_dot );
  int fat_directory_extract_name_short( fat_structure_directory_entry_t* entry, char* name );
  int fat_directory_extend( fat_directory_t* dir, void* buffer, uint64_t size );
  int fat_directory_dentry_insert( fat_directory_t* dir, const char* name, uint64_t cluster, bool directory );
  int fat_directory_dentry_update( fat_directory_t* dir, fat_structure_directory_entry_t* dentry, uint64_t pos );
  int fat_directory_dentry_remove( fat_directory_t* dir, fat_structure_directory_entry_t* dentry, uint64_t pos );
  int fat_directory_dentry_checksum( uint8_t* name, uint8_t* checksum );
#endif

#ifdef __cplusplus
}
#endif

#endif
