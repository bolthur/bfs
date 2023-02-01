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

/** @file fat/cluster.h */

#include <stdint.h>
#include <fat/fs.h>

#ifndef _FAT_CLUSTER_H
#define _FAT_CLUSTER_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Macro containing representation of an unused cluster */
#define FAT_CLUSTER_UNUSED 0
/** @brief EOF entry, used when inserting new clusters as used */
#define FAT_CLUSTER_EOF 0xFFFFFFFF
/** @brief FAT12 bad cluster identifier */
#define FAT_FAT12_BAD_CLUSTER 0xFF7
/** @brief FAT12 cluster chain end identifier */
#define FAT_FAT12_CLUSTER_CHAIN_END 0xFF8
/** @brief FAT16 bad cluster identifier */
#define FAT_FAT16_BAD_CLUSTER 0xFFF7
/** @brief FAT16 cluster chain end identifier */
#define FAT_FAT16_CLUSTER_CHAIN_END 0xFFF8
/** @brief FAT32 bad cluster identifier */
#define FAT_FAT32_BAD_CLUSTER 0x0FFFFFF7
/** @brief FAT32 cluster chain end identifier */
#define FAT_FAT32_CLUSTER_CHAIN_END 0x0FFFFFF8

#if defined( _BFS_COMPILING )
  int fat_cluster_next( fat_fs_t* fs, uint64_t current, uint64_t* next );
  int fat_cluster_to_lba( fat_fs_t* fs, uint64_t cluster, uint64_t* lba );
  int fat_cluster_get_free( fat_fs_t* fs, uint64_t* cluster );
  int fat_cluster_set_cluster( fat_fs_t* fs, uint64_t cluster, uint64_t value );
  int fat_cluster_get_by_num( fat_fs_t* fs, uint64_t cluster, uint64_t num, uint64_t* target );
  int fat_cluster_get_chain_end_value( fat_fs_t* fs, uint64_t* end );
#endif

#ifdef __cplusplus
}
#endif

#endif
