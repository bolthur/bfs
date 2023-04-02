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
#include <ext/directory.h>

int ext_directory_remove( const char* path ) {
  ( void )path;
  return ENOTSUP;
}

int ext_directory_move( const char* old_path, const char* new_path ) {
  ( void )old_path;
  ( void )new_path;
  return ENOTSUP;
}

int ext_directory_make( const char* path ) {
  ( void )path;
  return ENOTSUP;
}

int ext_directory_open( ext_directory_t* dir, const char* path ) {
  ( void )dir;
  ( void )path;
  return ENOTSUP;
}

int ext_directory_close( ext_directory_t* dir ) {
  ( void )dir;
  return ENOTSUP;
}

int ext_directory_next_entry( ext_directory_t* dir ) {
  ( void )dir;
  return ENOTSUP;
}

int ext_directory_rewind( ext_directory_t* dir ) {
  ( void )dir;
  return ENOTSUP;
}

int ext_directory_entry_by_name( ext_directory_t* dir, const char* path ) {
  ( void )dir;
  ( void )path;
  return ENOTSUP;
}
