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
#include <fat/block.h>
#include <fat/bfsfat_export.h>

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
  dir->file.fpos = 0;
  it->reference = dir;
  it->entry = NULL;
  it->data = NULL;
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
  return fat_iterator_directory_seek( it, it->reference->file.fpos + skip );
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
  // handle position exceeds size
  if ( pos >= it->reference->file.fsize ) {
    // free possible data
    if ( it->data ) {
      free( it->data );
      it->data = NULL;
    }
    // set fpos to position
    it->reference->file.fpos = pos;
    // return no entry
    return EOK;
  }
  // extract fs pointer
  fat_fs_t* fs = it->reference->file.mp->fs;
  // set iterator current to null as long as set isn't done
  it->entry = NULL;
  // cache block size
  uint64_t cluster_size = fs->superblock.sectors_per_cluster
    * fs->superblock.bytes_per_sector;
  // different cluster size for root directory for non fat32
  if ( it->reference->file.cluster == 0 && FAT_FAT32 != fs->type ) {
    cluster_size = fs->superblock.bytes_per_sector;
  }
  uint64_t next_block = pos / cluster_size;
  if (
    ! it->reference->file.block.data
    || it->reference->file.block.block != next_block
  ) {
    it->reference->file.fpos = pos;
    // trick fat block load to load next block correctly
    if ( 0 < next_block ) {
      it->reference->file.fpos += next_block * cluster_size;
    }
    // load block
    int result = fat_block_load( &it->reference->file, cluster_size );
    // validate return
    if ( EOK != result ) {
      return result;
    }
  }
  // update iterator position
  it->reference->file.fpos = pos;
  // return result of iterator set
  return fat_iterator_directory_set( it, cluster_size );
}

/**
 * @brief Fill iterator information with bunch of checks
 *
 * @param it
 * @param cluster_size
 * @return int
 */
BFSFAT_NO_EXPORT int fat_iterator_directory_set(
  fat_iterator_directory_t* it,
  uint64_t cluster_size
) {
  // validate parameter
  if ( ! it || ! cluster_size ) {
    return EINVAL;
  }
  uint64_t pos = it->reference->file.fpos;
  int result;
  // different cluster size for root directory for non fat32
  fat_fs_t* fs = it->reference->file.mp->fs;
  if ( it->reference->file.cluster == 0 && FAT_FAT32 != fs->type ) {
    cluster_size = fs->superblock.bytes_per_sector;
  }
  // allocate and fill data stuff
  if ( ! it->data ) {
    it->data = malloc( sizeof( fat_directory_data_t ) );
    if ( ! it->data ) {
      return ENOMEM;
    }
  }
  memset( it->data, 0, sizeof( fat_directory_data_t ) );

  fat_structure_directory_entry_t* entry = NULL;
  // skip all invalid entries
  while ( true ) {
    // cache block size
    uint64_t next_block = pos / cluster_size;
    uint64_t offset_within_block = pos % cluster_size;
    if (
      ! it->reference->file.block.data
      || it->reference->file.block.block != next_block
    ) {
      it->reference->file.fpos = pos;
      // trick fat block load to load next block correctly
      if ( 0 < next_block ) {
        it->reference->file.fpos += next_block * cluster_size;
      }
      // load block
      result = fat_block_load( &it->reference->file, cluster_size );
      // validate return
      if ( EOK != result ) {
        return result;
      }
    }
    // update iterator position
    it->reference->file.fpos = pos;
    // handle no valid one found ( end reached )
    if ( ! it->reference->file.block.data ) {
      return EOK;
    }
    // get directory entry
    entry = ( fat_structure_directory_entry_t* )(
      it->reference->file.block.data + offset_within_block
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
    // handle position exceeds size
    if ( pos >= it->reference->file.fsize ) {
      // free possible data
      if ( it->data ) {
        free( it->data );
        it->data = NULL;
      }
      // set fpos to position
      it->reference->file.fpos = pos;
      // return no entry
      return EOK;
    }
  }

  entry = NULL;
  fat_structure_directory_entry_long_t* entry_long = NULL;
  // get correct entry
  while ( true ) {
    // cache block size
    uint64_t next_block = pos / cluster_size;
    uint64_t offset_within_block = pos % cluster_size;
    if (
      ! it->reference->file.block.data
      || it->reference->file.block.block != next_block
    ) {
      it->reference->file.fpos = pos;
      // trick fat block load to load next block correctly
      if ( 0 < next_block ) {
        it->reference->file.fpos += next_block * cluster_size;
      }
      // load block
      result = fat_block_load( &it->reference->file, cluster_size );
      // validate return
      if ( EOK != result ) {
        return result;
      }
      it->reference->file.fpos = pos;
    }
    // update iterator position
    it->reference->file.fpos = pos;
    // handle no valid one found ( end reached )
    if ( ! it->reference->file.block.data ) {
      return EOK;
    }
    // get directory entry
    entry = ( fat_structure_directory_entry_t* )(
      it->reference->file.block.data + offset_within_block
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
      uint64_t long_index = ( order - 1U ) * 13U;
      // fill entry
      for ( uint64_t idx = 0; idx < 10; idx += 2 ) {
        it->data->name[ long_index++ ] = ( char )entry_long->first_five_two_byte[ idx ];
      }
      for ( uint64_t idx = 0; idx < 12; idx += 2 ) {
        it->data->name[ long_index++ ] = ( char )entry_long->next_six_two_byte[ idx ];
      }
      for ( uint64_t idx = 0; idx < 4; idx += 2 ) {
        it->data->name[ long_index++ ] = ( char )entry_long->final_two_byte[ idx ];
      }
    } else if ( FAT_DIRECTORY_ENTRY_DOT_ENTRY == entry->name[ 0 ] ) {
      uint64_t idx = 0;
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
  // update position
  it->reference->file.fpos = pos;
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
  // return success
  return EOK;
}
