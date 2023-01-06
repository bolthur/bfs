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

#include <stdint.h>
#include <fat/fs.h>

#ifndef _FAT_CLUSTER_H
#define _FAT_CLUSTER_H

#ifdef __cplusplus
extern "C" {
#endif

#define FAT_FAT12_BAD_CLUSTER 0xFF7
#define FAT_FAT12_CLUSTER_CHAIN_END 0xFF8

#define FAT_FAT16_BAD_CLUSTER 0xFFF7
#define FAT_FAT16_CLUSTER_CHAIN_END 0xFFF8

#define FAT_FAT32_BAD_CLUSTER 0x0FFFFFF7
#define FAT_FAT32_CLUSTER_CHAIN_END 0x0FFFFFF8

int fat_cluster_next( fat_fs_t* fs, uint64_t current, uint64_t* next );
int fat_cluster_to_lba( fat_fs_t* fs, uint64_t cluster, uint64_t* lba );

#ifdef __cplusplus
}
#endif

#endif
