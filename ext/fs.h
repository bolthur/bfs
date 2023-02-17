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

/** @file ext/fs.h */

#include <stdbool.h>
#include <stdint.h>
#include <common/blockdev.h>
#include <ext/structure.h>

#ifndef _EXT_FS_H
#define _EXT_FS_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Fat file system object */
typedef struct {
  /** @brief Flag indicating access is read only or not */
  bool read_only;
  /** @brief Used block device for access */
  common_blockdev_t* bdev;
  /** @brief super block information */
  ext_structure_superblock_t superblock;
} ext_fs_t;


#if defined( _BFS_COMPILING )
  int ext_fs_init( ext_fs_t* fs, common_blockdev_t* bdev, bool read_only );
  int ext_fs_fini( ext_fs_t* fs );
  int ext_fs_check_feature( ext_fs_t* fs );
#endif

#ifdef __cplusplus
}
#endif

#endif
