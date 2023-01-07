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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <common/blockdev.h>
#include <common/errno.h> // IWYU pragma: keep
#include <common/mountpoint.h>
#include <fat/rootdir.h>
#include <fat/directory.h>
#include <fat/type.h>
#include <fat/fs.h>
#include <fat/file.h>
#include <fat/structure.h>
#include <fat/bfsfat_export.h>

/**
 * @brief Method to open root directory
 *
 * @param mp
 * @param dir
 * @return int
 */
BFSFAT_NO_EXPORT int fat_rootdir_open(
  common_mountpoint_t *mp,
  fat_directory_t* dir
) {
  // validate parameter
  if ( ! dir || ! mp ) {
    return EINVAL;
  }
  // populate mountpoint
  dir->file.mp = mp;
  // variables for root dir size
  uint64_t rootdir_size;
  uint64_t rootdir_offset;
  // query root dir size and offset
  int result = fat_rootdir_offset_size( dir, &rootdir_offset, &rootdir_size );
  if ( EOK != result ) {
    dir->file.mp = NULL;
    return result;
  }
  // cache block size
  fat_fs_t* fs = mp->fs;
  // populate directory
  dir->file.cluster = FAT_FAT32 == fs->type
    ? ( uint32_t )rootdir_offset : 0;
  dir->file.fpos = 0;
  dir->file.fsize = rootdir_size * fs->bdev->bdif->block_size;
  // return success
  return EOK;
}

/**
 * @brief Close openend root directory
 *
 * @param dir
 * @return int
 */
BFSFAT_NO_EXPORT int fat_rootdir_close( fat_directory_t* dir ) {
  // validate parameter
  if ( ! dir ) {
    return EINVAL;
  }
  if ( dir->data ) {
    free( dir->data );
  }
  if ( dir->entry ) {
    free( dir->entry );
  }
  // close file
  int result = fat_file_close( &dir->file );
  if ( EOK != result ) {
    return result;
  }
  // overwrite everything with 0
  memset( dir, 0, sizeof( *dir ) );
  // return success
  return EOK;
}

/**
 * @brief Get root directory offset and size
 *
 * @param dir
 * @param offset
 * @param size
 * @return int
 */
BFSFAT_NO_EXPORT int fat_rootdir_offset_size(
  fat_directory_t* dir,
  uint64_t* offset,
  uint64_t* size
) {
  // validate
  if ( ! dir || ! dir->file.mp->fs || ! offset || ! size ) {
    return EINVAL;
  }
  // bunch of necessary variables
  fat_fs_t* fs = dir->file.mp->fs;
  uint64_t rootdir_offset;
  uint64_t rootdir_size;
  uint64_t block_size = fs->bdev->bdif->block_size;
  // handle different types
  if ( FAT_FAT12 == fs->type || FAT_FAT16 == fs->type ) {
    // FAT12 and FAT16 have fixed offset and root directory size
    rootdir_offset = ( uint64_t )( fs->superblock.reserved_sector_count + (
      fs->superblock.table_count * fs->superblock.table_size_16
    ) );
    // calculate root dir size with round up
    rootdir_size = (
      fs->superblock.root_entry_count
        * sizeof( fat_structure_directory_entry_t )
        + ( block_size - 1 )
    ) / 512;
  } else if ( FAT_FAT32 == fs->type ) {
    // root dir in fat32 is stored within data section
    dir->file.cluster = fs->superblock.extended.fat32.root_cluster;
    // gather size of root directory like with a normal directory
    int result = fat_directory_size( dir, &rootdir_size );
    if ( EOK != result ) {
      return result;
    }
    // set rootdir offset
    rootdir_offset = dir->file.cluster;
  } else {
    return EINVAL;
  }
  // populate data
  *offset = rootdir_offset;
  *size = rootdir_size;
  // return success
  return EOK;
}
