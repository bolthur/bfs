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
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <common/errno.h>
#include <common/blockdev.h>
#include <common/mountpoint.h>
#include <fat/type.h>
#include <fat/iterator.h>
#include <fat/cluster.h>
#include <fat/directory.h>
#include <fat/fs.h>
#include <fat/rootdir.h>
#include <fat/structure.h>
#include <fat/bfsfat_export.h>

static int fat_directory_iterator_load_next_block(
  fat_iterator_directory_t* it,
  uint64_t current_block,
  uint64_t next_block,
  uint64_t pos
) {
  // get reference data
  fat_directory_t* dir = it->reference;
  // extract fs pointer
  fat_fs_t* fs = it->reference->file.mp->fs;
  // extract block device
  common_blockdev_t* bdev = fs->bdev;
  // check whether end is reached
  if ( pos >= dir->file.fsize ) {
    // clear block data if set
    if ( it->block.data ) {
      free( it->block.data );
      // reset sector to 0 and data
      it->block.data = NULL;
      it->block.sector = 0;
    }
    // set offset
    it->offset = pos;
    return EOK;
  }
  // handle no sector loaded or different blocks
  if ( ! it->block.data || current_block != next_block ) {
    // free up set block
    if ( it->block.data ) {
      // free up block data
      free( it->block.data );
      // reset sector to 0 and data
      it->block.data = NULL;
      it->block.sector = 0;
    }
    // get next block index
    if ( 0 == it->reference->file.cluster ) {
      // next root block is root directory offset + sector
      uint64_t rootdir_offset;
      uint64_t rootdir_size;
      // get offset and size of root directory
      int result = fat_rootdir_offset_size( dir, &rootdir_offset, &rootdir_size );
      if ( EOK != result ) {
        return result;
      }
      // allocate buffer if not allocated
      if ( ! it->block.data ) {
        // allocate block
        it->block.data = malloc( bdev->bdif->block_size );
        if ( ! it->block.data ) {
          return ENOMEM;
        }
        // clear out block
        memset( it->block.data, 0, bdev->bdif->block_size );
      }
      // read bytes from device
      result = common_blockdev_bytes_read(
        bdev,
        ( rootdir_offset + next_block ) * bdev->bdif->block_size,
        it->block.data,
        bdev->bdif->block_size
      );
      // handle error
      if ( EOK != result ) {
        return result;
      }
    } else {
      // allocate buffer if not allocated
      if ( ! it->block.data ) {
        // allocate block
        it->block.data = malloc( bdev->bdif->block_size );
        if ( ! it->block.data ) {
          return ENOMEM;
        }
        // clear out block
        memset( it->block.data, 0, bdev->bdif->block_size );
      }
      uint64_t lba;
      // transform data cluster to lba
      int result = fat_cluster_to_lba( fs, it->reference->file.cluster, &lba );
      if ( EOK != result ) {
        return result;
      }
      // read bytes from device
      result = common_blockdev_bytes_read(
        bdev,
        lba * bdev->bdif->block_size,
        it->block.data,
        bdev->bdif->block_size
      );
      // handle error
      if ( EOK != result ) {
        return result;
      }
    }
  }
  // set offset
  it->offset = pos;
  // return success
  return EOK;
}

/**
 * @brief Direcotry iterator init
 *
 * @param it
 * @param dir
 * @param pos
 * @return int
 */
BFSFAT_NO_EXPORT int fat_iterator_directory_init(
  fat_iterator_directory_t* it,
  fat_directory_t* dir,
  uint64_t pos
) {
  // validate parameter
  if ( ! it || ! dir ) {
    return EINVAL;
  }
  // set reference
  it->reference = dir;
  it->entry = NULL;
  it->data = NULL;
  it->offset = 0;
  it->block.sector = 0;
  // return result of seek
  return fat_iterator_directory_seek( it, pos );
}

/**
 * @brief Directory iterator next
 *
 * @param it
 * @return int
 */
BFSFAT_NO_EXPORT int fat_iterator_directory_next( fat_iterator_directory_t* it ) {
  // skip value is sizeof entry structure
  uint64_t skip = sizeof( fat_structure_directory_entry_t );
  // get next directory
  return fat_iterator_directory_seek( it, it->offset + skip );
}

/**
 * @brief Perform seek
 *
 * @param it
 * @param pos
 * @return int
 */
BFSFAT_NO_EXPORT int fat_iterator_directory_seek(
  fat_iterator_directory_t* it,
  uint64_t pos
) {
  // validate parameter
  if ( ! it ) {
    return EINVAL;
  }
  // extract fs pointer
  fat_fs_t* fs = it->reference->file.mp->fs;
  // extract block device
  common_blockdev_t* bdev = fs->bdev;
  // set iterator current to null as long as set isn't done
  it->entry = NULL;
  // cache block size
  uint64_t block_size = bdev->bdif->block_size;
  uint64_t current_block = it->offset / block_size;
  uint64_t next_block = pos / block_size;
  // load next block
  int result = fat_directory_iterator_load_next_block(
    it, current_block, next_block, pos );
  if ( EOK != result ) {
    return result;
  }
  // return result of iterator set
  return fat_iterator_directory_set( it, block_size );
}

/**
 * @brief Fill iterator information with bunch of checks
 *
 * @param it
 * @param block_size
 * @return int
 */
BFSFAT_NO_EXPORT int fat_iterator_directory_set(
  fat_iterator_directory_t* it,
  uint64_t block_size
) {
  // validate parameter
  if ( ! it || ! block_size ) {
    return EINVAL;
  }
  uint64_t pos = it->offset;

  // allocate and fill data stuff
  if ( ! it->data ) {
    it->data = malloc( sizeof( fat_directory_data_t ) );
    if ( ! it->data ) {
      return ENOMEM;
    }
    memset( it->data, 0, sizeof( fat_directory_data_t ) );
  }

  fat_structure_directory_entry_t* entry = NULL;
  // skip all invalid entries
  while ( true ) {
    // cache block size
    uint64_t current_block = it->offset / block_size;
    uint64_t next_block = pos / block_size;
    uint64_t offset_within_block = pos % block_size;
    // load next block if necessary
    int result = fat_directory_iterator_load_next_block(
      it, current_block, next_block, pos );
    if ( EOK != result ) {
      return EOK;
    }
    // handle no valid one found ( end reached )
    if ( ! it->block.data ) {
      return EOK;
    }
    // get directory entry
    entry = ( fat_structure_directory_entry_t* )(
      it->block.data + offset_within_block
    );
    // valid flag
    bool is_valid;
    // check whether entry is valid
    result = fat_directory_entry_is_valid( entry, &is_valid );
    // handle error
    if ( EOK != result ) {
      free( it->data );
      it->data = NULL;
      return result;
    }
    // handle valid found
    if ( is_valid ) {
      break;
    }
    // increment position
    pos += sizeof( fat_structure_directory_entry_t );
  }

  entry = NULL;
  fat_structure_directory_entry_long_t* entry_long = NULL;
  // get correct entry
  while ( true ) {
    // cache block size
    uint64_t current_block = it->offset / block_size;
    uint64_t next_block = pos / block_size;
    uint64_t offset_within_block = pos % block_size;
    // load next block if necessary
    int result = fat_directory_iterator_load_next_block(
      it, current_block, next_block, pos );
    if ( EOK != result ) {
      return EOK;
    }
    // handle no valid one found ( end reached )
    if ( ! it->block.data ) {
      return EOK;
    }
    // get directory entry
    entry = ( fat_structure_directory_entry_t* )(
      it->block.data + offset_within_block
    );
    // push filename with extension to data
    if ( FAT_DIRECTORY_FILE_ATTRIBUTE_LONG_FILE_NAME == entry->attributes ) {
      // transform to long entry
      entry_long = ( fat_structure_directory_entry_long_t* )entry;
      // extract order
      uint8_t order = entry_long->order;
      // handle first one
      if ( ( order | 0x40 ) == order ) {
        // clear name
        memset( it->data->name, 0, sizeof( char ) * NAME_MAX );
        // extract order position
        order -= 0x40;
      }
      // calculate index start by order
      size_t long_index = ( ( size_t )order - 1 ) * 13;
      // fill entry
      for ( size_t idx = 0; idx < 10; idx += 2 ) {
        it->data->name[ long_index++ ] = ( char )entry_long->first_five_two_byte[ idx ];
      }
      for ( size_t idx = 0; idx < 12; idx += 2 ) {
        it->data->name[ long_index++ ] = ( char )entry_long->next_six_two_byte[ idx ];
      }
      for ( size_t idx = 0; idx < 4; idx += 2 ) {
        it->data->name[ long_index++ ] = ( char )entry_long->final_two_byte[ idx ];
      }
    } else if ( FAT_DIRECTORY_ENTRY_DOT_ENTRY == entry->name[ 0 ] ) {
      size_t idx = 0;
      // set first dot
      it->data->name[ idx++ ] = entry->name[ 0 ];
      // check for second dot
      if ( entry->name[ 1 ] == '.' ) {
        it->data->name[ idx++ ] = '.';
      }
      // add null-termination
      it->data->name[ idx ] = '\0';
      // break
      break;
    } else {
      if ( ! entry_long ) {
        // extract short name
        result = fat_directory_extract_name_short( entry, it->data->name );
        if ( EOK != result ) {
          return result;
        }
      }
      // break out
      break;
    }
    // increment position
    pos += sizeof( fat_structure_directory_entry_t );
  }

  // push entry
  it->entry = entry;
  // return success
  return EOK;
}

/**
 * @brief directory iterator finish
 *
 * @param it
 * @return int
 */
BFSFAT_NO_EXPORT int fat_iterator_directory_fini( fat_iterator_directory_t* it ) {
  // validate parameter
  if ( ! it ) {
    return EINVAL;
  }
  // reset current
  it->entry = NULL;
  // clear data
  if ( it->data ) {
    free( it->data );
    it->data = NULL;
  }
  // cleanup block data
  if ( it->block.data ) {
    // free up block data
    free( it->block.data );
    // reset sector to 0 and data
    it->block.data = NULL;
    it->block.sector = 0;
  }
  // return success
  return EOK;
}
