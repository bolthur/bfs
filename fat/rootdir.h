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

/** @file fat/rootdir.h */

#include <stdint.h>
#include <common/mountpoint.h>
#include <fat/type.h>

#ifndef _FAT_ROOTDIR_H
#define _FAT_ROOTDIR_H

#ifdef __cplusplus
extern "C" {
#endif

int fat_rootdir_open( common_mountpoint_t *mp, fat_directory_t* dir );
int fat_rootdir_close( fat_directory_t* dir );
int fat_rootdir_offset_size( fat_directory_t* dir, uint64_t* offset, uint64_t* size );

#ifdef __cplusplus
}
#endif

#endif
