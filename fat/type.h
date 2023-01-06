/**
 * Copyright (C) 2022 bolthur project.
 *
 * This file is part of bolthur/bfs.
 *
 * bolthur/bfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bolthur/bfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bolthur/bfs.  If not, see <http://www.gnu.org/licenses/>.
 */

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

typedef struct fat_directory_data fat_directory_data_t;

typedef struct {
  uint64_t sector;
  uint8_t* data;
} fat_block_t;

typedef struct fat_file {
  common_mountpoint_t *mp;
  uint32_t flags;
  uint64_t fsize;
  uint64_t fpos;
  uint32_t cluster;
} fat_file_t;

typedef struct {
  fat_file_t file;
  fat_structure_directory_entry_t* entry;
  fat_directory_data_t* data;
} fat_directory_t;

int fat_type_validate( fat_fs_t* fs );

#ifdef __cplusplus
}
#endif

#endif
