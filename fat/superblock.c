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

// IWYU pragma: no_include <errno.h>
#include <stdint.h>
#include <common/blockdev.h>
#include <common/errno.h>
#include <fat/superblock.h>
#include <fat/fs.h>
#include <fat/structure.h>
#include <fat/bfsfat_export.h>

/**
 * @brief Read superblock
 *
 * @param bdev
 * @param superblock
 * @return int
 */
BFSFAT_EXPORT int fat_superblock_read(
  common_blockdev_t* bdev,
  fat_structure_superblock_t* superblock
) {
  return common_blockdev_bytes_read(
    bdev,
    0,
    superblock,
    sizeof( fat_structure_superblock_t )
  );
}

/**
 * @brief Validate
 *
 * @param fs
 * @return int
 */
BFSFAT_EXPORT int fat_superblock_check( fat_fs_t* fs ) {
  // validate parameter
  if ( ! fs ) {
    return EINVAL;
  }
  // check signature
  if ( fs->superblock.boot_sector_signature != 0xaa55 ) {
    return ENOTSUP;
  }
  // collect bunch of data
  fs->total_sectors = 0 == fs->superblock.total_sectors_16
    ? fs->superblock.total_sectors_32
    : fs->superblock.total_sectors_16;
  fs->fat_size = 0 == fs->superblock.table_size_16
    ? fs->superblock.extended.fat32.table_size_32
    : fs->superblock.table_size_16;
  fs->root_dir_sectors = ( uint32_t )( ( ( fs->superblock.root_entry_count * 32 )
    + ( fs->superblock.bytes_per_sector - 1 ) ) / fs->superblock.bytes_per_sector );
  fs->first_data_sector = fs->superblock.reserved_sector_count
    + fs->superblock.hidden_sector_count + (
      fs->superblock.table_count * fs->fat_size
    ) + fs->root_dir_sectors;
  fs->first_fat_sector = fs->superblock.reserved_sector_count;
  fs->data_sectors = fs->total_sectors - fs->first_data_sector;
  fs->total_clusters = fs->data_sectors / fs->superblock.sectors_per_cluster;
  // return everything fine
  return EOK;
}
