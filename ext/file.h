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

/** @file ext/file.h */

#include <stdint.h>
#include <ext/type.h>

#ifndef _EXT_FILE_H
#define _EXT_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

int ext_file_remove( const char* path );
int ext_file_move( const char* old_path, const char* new_path );
int ext_file_open( ext_file_t* file, const char* path, const char* flags );
int ext_file_open2( ext_file_t* file, const char* path, int flags );
int ext_file_close( ext_file_t* file );
int ext_file_truncate( ext_file_t* file, uint64_t size );
int ext_file_read( ext_file_t* file, void* buffer, uint64_t size, uint64_t* read_count );
int ext_file_write( ext_file_t* file, void* buffer, uint64_t size, uint64_t* write_count );
int ext_file_seek( ext_file_t* file, int64_t offset, uint32_t whence );
int ext_file_tell( ext_file_t* file, uint64_t* offset );
int ext_file_size( ext_file_t* file, uint64_t* size );

#if defined( _BFS_COMPILING )
  int fat_file_get( ext_file_t* file, const char* path, int flags );
#endif

#ifdef __cplusplus
}
#endif

#endif
