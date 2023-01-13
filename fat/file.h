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

/** @file fat/file.h */

#include <stddef.h>
#include <stdint.h>
#include <fat/type.h>

#ifndef _FAT_FILE_H
#define _FAT_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

int fat_file_remove( const char* path );
int fat_file_link( const char* path, const char* link_path );
int fat_file_move( const char* old_path, const char* new_path );
int fat_file_open( fat_file_t* file, const char* path, const char* flags );
int fat_file_open2( fat_file_t* file, const char* path, int flags );
int fat_file_close( fat_file_t* file );
int fat_file_truncate( fat_file_t* file, uint64_t size );
int fat_file_read( fat_file_t* file, void* buffer, uint64_t size, uint64_t* read_count );
int fat_file_write( fat_file_t* file, void* buffer, uint64_t size, uint64_t* write_count );
int fat_file_seek( fat_file_t* file, int64_t offset, uint32_t whence );
int fat_file_tell( fat_file_t* file, uint64_t* offset );
int fat_file_size( fat_file_t* file, uint64_t* size );

int fat_file_get( fat_file_t* file, const char* path, int flags );
int fat_file_extend_cluster( fat_file_t* file, uint64_t num );

#ifdef __cplusplus
}
#endif

#endif
