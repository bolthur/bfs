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

#include <common/errno.h>
#include <ext/file.h>

int ext_file_remove( const char* path ) {
  ( void )path;
  return ENOTSUP;
}

int ext_file_move( const char* old_path, const char* new_path ) {
  ( void )old_path;
  ( void )new_path;
  return ENOTSUP;
}

int ext_file_open( ext_file_t* file, const char* path, const char* flags ) {
  ( void )file;
  ( void )path;
  ( void )flags;
  return ENOTSUP;
}

int ext_file_open2( ext_file_t* file, const char* path, int flags ) {
  ( void )file;
  ( void )path;
  ( void )flags;
  return ENOTSUP;
}

int ext_file_close( ext_file_t* file ) {
  ( void )file;
  return ENOTSUP;
}

int ext_file_truncate( ext_file_t* file, uint64_t size ) {
  ( void )file;
  ( void )size;
  return ENOTSUP;
}

int ext_file_read(
  ext_file_t* file,
  void* buffer,
  uint64_t size,
  uint64_t* read_count
) {
  ( void )file;
  ( void )buffer;
  ( void )size;
  ( void )read_count;
  return ENOTSUP;
}

int ext_file_write(
  ext_file_t* file,
  void* buffer,
  uint64_t size,
  uint64_t* write_count
) {
  ( void )file;
  ( void )buffer;
  ( void )size;
  ( void )write_count;
  return ENOTSUP;
}

int ext_file_seek( ext_file_t* file, int64_t offset, uint32_t whence ) {
  ( void )file;
  ( void )offset;
  ( void )whence;
  return ENOTSUP;
}

int ext_file_tell( ext_file_t* file, uint64_t* offset ) {
  ( void )file;
  ( void )offset;
  return ENOTSUP;
}

int ext_file_size( ext_file_t* file, uint64_t* size ) {
  ( void )file;
  ( void )size;
  return ENOTSUP;
}

int fat_file_get( ext_file_t* file, const char* path, int flags ) {
  ( void )file;
  ( void )path;
  ( void )flags;
  return ENOTSUP;
}
