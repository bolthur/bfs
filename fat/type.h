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

/** @file fat/type.h */

// IWYU pragma: no_include <stdio.h>
#include <stdint.h>
#include <common/stdio.h>
#include <common/mountpoint.h>
#include <fat/structure.h>
#include <fat/fs.h>

#ifndef _FAT_TYPE_H
#define _FAT_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Container for additional fat directory data */
typedef struct {
  /** @brief Whole fat directory name */
  char name[ NAME_MAX ];
} fat_directory_data_t;

/** @brief Fat block with sector information and data */
typedef struct {
  /** @brief Sector where data is from */
  uint64_t sector;
  /** @brief Cluster where data is from */
  uint64_t cluster;
  /** @brief Block in cluster chain where data is from */
  uint64_t block;
  /** @brief Data from sector */
  uint8_t* data;
} fat_block_t;

typedef struct fat_directory fat_directory_t;

/** @brief Fat file definition */
typedef struct fat_file {
  /** @brief Mount point this file is related to */
  common_mountpoint_t *mp;
  /** @brief Flags used while opening the file */
  uint32_t flags;
  /** @brief File size in bytes */
  uint64_t fsize;
  /** @brief Current position in file */
  uint64_t fpos;
  /** @brief File start cluster */
  uint32_t cluster;
  /** @brief Some data block used for reading */
  fat_block_t block;
  /** @brief Directory containing the file */
  fat_directory_t* dir;
  /** @brief fat directory entry of file */
  fat_structure_directory_entry_t* dentry;
  /** @brief dentry position in bytes */
  uint64_t dentry_pos;
} fat_file_t;

/** @brief Fat directory structure */
typedef struct fat_directory {
  /** @brief File instance used for accessing clusters */
  fat_file_t file;
  /** @brief Current fat directory entry */
  fat_structure_directory_entry_t* entry;
  /** @brief Current fat directory entry offset */
  uint64_t entry_pos;
  /** @brief directory content size */
  uint64_t entry_size;
  /** @brief Current fat directory data */
  fat_directory_data_t* data;
} fat_directory_t;

int fat_type_validate( fat_fs_t* fs );

#ifdef __cplusplus
}
#endif

#endif
