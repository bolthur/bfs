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

#ifndef _HELPER_HH
#define _HELPER_HH

#include <fat/structure.h>

#ifdef __cplusplus
extern "C" {
#endif

void helper_mount_test_image( bool read_only, const char* fname, const char* device, const char* path, fat_type_t );
void helper_unmount_test_image( const char* device, const char* path );

#ifdef __cplusplus
}
#endif

#endif
