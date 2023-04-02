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
#include <ext/info.h>

int ext_info_ctime( const char* path, time_t* ctime ) {
  ( void )path;
  ( void )ctime;
  return ENOTSUP;
}

int ext_info_mtime( const char* path, time_t* mtime ) {
  ( void )path;
  ( void )mtime;
  return ENOTSUP;
}

int ext_info_atime( const char* path, time_t* atime ) {
  ( void )path;
  ( void )atime;
  return ENOTSUP;
}

int ext_info_mode( const char* path, uint64_t* mode ) {
  ( void )path;
  ( void )mode;
  return ENOTSUP;
}

int ext_info_owner( const char* path, uint64_t* owner ) {
  ( void )path;
  ( void )owner;
  return ENOTSUP;
}

int ext_info_link_count( const char* path, uint64_t* count ) {
  ( void )path;
  ( void )count;
  return ENOTSUP;
}
