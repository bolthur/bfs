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

// IWYU pragma: no_include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <common/blockdev.h>
#include <common/errno.h> // IWYU pragma: keep
#include <common/mountpoint.h>
#include <fat/rootdir.h>
#include <fat/directory.h>
#include <fat/type.h>
#include <fat/fs.h>
#include <fat/file.h>
#include <fat/block.h>
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
  dir->file.fsize = FAT_FAT32 == fs->type
    ? rootdir_size : rootdir_size * fs->bdev->bdif->block_size;
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
    ) / block_size;
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

/**
 * @brief Helper to extend root directory
 *
 * @param dir
 * @param buffer
 * @param size
 * @return int
 */
BFSFAT_NO_EXPORT int fat_rootdir_extend( fat_directory_t* dir, void* buffer, uint64_t size ) {
  // validate
  if ( ! dir || ! dir->file.mp->fs || ! buffer || ! size ) {
    return EINVAL;
  }
  // bunch of necessary variables
  fat_fs_t* fs = dir->file.mp->fs;
  int result;
  // check for readonly
  if ( fs->read_only ) {
    return EROFS;
  }
  uint64_t block_size = fs->bdev->bdif->block_size;
  uint64_t necessary_entry_count = size / sizeof( fat_structure_directory_entry_t );
  // root dir of fat12 and 16 fixed sized
  if ( FAT_FAT12 == fs->type || FAT_FAT16 == fs->type ) {
    // FAT12 and FAT16 have fixed offset and root directory size
    uint64_t rootdir_offset = ( uint64_t )( fs->superblock.reserved_sector_count + (
      fs->superblock.table_count * fs->superblock.table_size_16
    ) );
    // calculate root dir size with round up
    uint64_t rootdir_size = (
      fs->superblock.root_entry_count
        * sizeof( fat_structure_directory_entry_t )
        + ( block_size - 1 )
    ) / block_size;
    fat_structure_directory_entry_t* entry = malloc(
      ( size_t )( rootdir_size * block_size ) );
    if ( ! entry ) {
      return ENOMEM;
    }
    // load root directory block by block
    uint64_t fpos = 0;
    uint64_t fsize = rootdir_size * block_size;
    while ( fpos < fsize ) {
      // load fat block
      result = common_blockdev_bytes_read(
        fs->bdev,
        ( rootdir_offset + fpos / block_size ) * block_size,
        ( uint8_t* )entry + fpos,
        block_size
      );
      if ( EOK != result ) {
        free( entry );
        return result;
      }
      fpos += block_size;
    }
    // loop through root directory and try to find a entry
    fat_structure_directory_entry_t* current = entry;
    fat_structure_directory_entry_t* end = ( fat_structure_directory_entry_t* )(
      ( uint8_t* )entry + rootdir_size * block_size
    );
    fat_structure_directory_entry_t* start = NULL;
    uint64_t found_size = 0;
    while ( current < end ) {
      // handle found enough space
      if ( start && found_size == necessary_entry_count ) {
        break;
      }
      bool is_free;
      result = fat_directory_entry_is_free( current, &is_free );
      if ( EOK != result ) {
        free( entry );
        return ENOSPC;
      }
      // handle non free
      if ( ! is_free ) {
        found_size = 0;
        start = NULL;
      // handle free block
      } else if ( is_free ) {
        if ( ! start ) {
          start = current;
        }
        found_size++;
      }
      // increment current
      current++;
    }
    // handle not enough free stuff
    if ( ! start || found_size != necessary_entry_count ) {
      free( entry );
      return ENOSPC;
    }
    // copy over changes
    memcpy( start, buffer, ( size_t )size );
    // write root directory block by block
    fpos = 0;
    fsize = rootdir_size * block_size;
    while ( fpos < fsize ) {
      // load fat block
      result = common_blockdev_bytes_write(
        fs->bdev,
        ( rootdir_offset + fpos / block_size ) * block_size,
        ( uint8_t* )entry + fpos,
        block_size
      );
      if ( EOK != result ) {
        free( entry );
        return result;
      }
      fpos += block_size;
    }
    // free entry and return success
    free( entry );
    return EOK;
  } else if ( FAT_FAT32 == fs->type ) {
    // fat32 root directory is a normal directory
    // overwrite cluster
    dir->file.cluster = fs->superblock.extended.fat32.root_cluster;
    // try to extend directory
    return fat_directory_extend( dir, buffer, size );
  } else {
    return EINVAL;
  }
}

/**
 * @brief Remove dir entry from root director
 *
 * @param dir
 * @param dentry
 * @param pos
 * @return int
 */
BFSFAT_NO_EXPORT int fat_rootdir_remove(
  fat_directory_t* dir,
  fat_structure_directory_entry_t* dentry,
  uint64_t pos
) {
  // validate
  if ( ! dir || ! dir->file.mp->fs || ! dentry ) {
    return EINVAL;
  }
  // bunch of necessary variables
  fat_fs_t* fs = dir->file.mp->fs;
  // check for readonly
  if ( fs->read_only ) {
    return EROFS;
  }
  // fat32 root directory is treated as normal directory
  if ( FAT_FAT32 == fs->type ) {
    dir->file.cluster = fs->superblock.extended.fat32.root_cluster;
    return fat_directory_dentry_remove( dir, dentry, pos );
  }
  // handle unsupported
  if ( FAT_FAT12 != fs->type && FAT_FAT16 != fs->type ) {
    return EINVAL;
  }
  uint64_t block_size = fs->bdev->bdif->block_size;
  uint64_t rootdir_offset = ( uint64_t )( fs->superblock.reserved_sector_count + (
    fs->superblock.table_count * fs->superblock.table_size_16
  ) );
  // calculate root dir size with round up
  uint64_t rootdir_size = (
    fs->superblock.root_entry_count
      * sizeof( fat_structure_directory_entry_t )
      + ( block_size - 1 )
  ) / block_size;
  // validate pos
  if ( pos > rootdir_size * block_size ) {
    return EINVAL;
  }
  // allocate space
  fat_structure_directory_entry_t* entry = malloc(
    ( size_t )( rootdir_size * block_size ) );
  if ( ! entry ) {
    return ENOMEM;
  }
  // load whole root directory
  int result = common_blockdev_bytes_read(
    fs->bdev,
    rootdir_offset * block_size,
    entry,
    rootdir_size * block_size
  );
  if ( EOK != result ) {
    free( entry );
    return result;
  }
  fat_structure_directory_entry_t* start = entry;
  fat_structure_directory_entry_t* current = ( fat_structure_directory_entry_t* )(
    ( uint8_t* )entry + pos
  );
  uint64_t count = 0;
  // loop backwards until first possible long name
  while( true ) {
    // decrement current
    current--;
    // increment count if long name
    if ( current->attributes == FAT_DIRECTORY_FILE_ATTRIBUTE_LONG_FILE_NAME ) {
      count++;
    }
    // handle beginning reached
    if (
      start >= current
      || current->attributes != FAT_DIRECTORY_FILE_ATTRIBUTE_LONG_FILE_NAME
    ) {
      break;
    }
  }
  // update position
  pos -= count * sizeof( fat_structure_directory_entry_t );
  // increment due to short entry
  count++;
  // clear out
  memset(
    ( uint8_t* )start + pos,
    0,
    ( size_t )( count * sizeof( fat_structure_directory_entry_t ) )
  );
  // write back whole root directory
  result = common_blockdev_bytes_write(
    fs->bdev,
    rootdir_offset * block_size,
    entry,
    rootdir_size * block_size
  );
  if ( EOK != result ) {
    free( entry );
    return result;
  }
  free( entry );
  return EOK;
}
