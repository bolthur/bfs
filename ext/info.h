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

/** @file ext/info.h */

#include <time.h>
#include <stdint.h>
#include <ext/type.h>

#ifndef _EXT_INFO_H
#define _EXT_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

int ext_info_ctime( const char* path, time_t* ctime );
int ext_info_mtime( const char* path, time_t* mtime );
int ext_info_atime( const char* path, time_t* atime );
int ext_info_mode( const char* path, uint64_t* mode );
int ext_info_owner( const char* path, uint64_t* owner );
int ext_info_link_count( const char* path, uint64_t* count );

#ifdef __cplusplus
}
#endif

#endif
