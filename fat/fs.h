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

/** @file fat/fs.h */

#include <stdbool.h>
#include <stdint.h>
#include <common/blockdev.h>
#include <fat/structure.h>

#ifndef _FAT_FS_H
#define _FAT_FS_H

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
  fat_structure_superblock_t superblock;
  /** @brief Evaluated fat type */
  fat_type_t type;
  /** @brief Total sector count */
  uint32_t total_sectors;
  /** @brief Fat table size */
  uint32_t fat_size;
  /** @brief Amount of root directory sectors */
  uint32_t root_dir_sectors;
  /** @brief First data sector */
  uint32_t first_data_sector;
  /** @brief First fat sector */
  uint32_t first_fat_sector;
  /** @brief Amount of data sectors */
  uint32_t data_sectors;
  /** @brief Amount of total clusters */
  uint32_t total_clusters;
} fat_fs_t;


#if defined( _BFS_COMPILING )
  int fat_fs_init( fat_fs_t* fs, common_blockdev_t* bdev, bool read_only );
  int fat_fs_fini( fat_fs_t* fs );
#endif

#ifdef __cplusplus
}
#endif

#endif
