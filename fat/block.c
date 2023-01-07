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
#include <string.h>
#include <stdlib.h>
#include <common/errno.h>
#include <fat/type.h>
#include <fat/fs.h>
#include <fat/block.h>
#include <fat/cluster.h>
#include <fat/rootdir.h>

/**
 * @brief Method to load a fat block by file offset
 *
 * @param file
 * @return int
 */
int fat_block_load( fat_file_t* file ) {
  if ( ! file ) {
    return EINVAL;
  }
  // extract fs pointer
  fat_fs_t* fs = file->mp->fs;
  // extract block device
  common_blockdev_t* bdev = fs->bdev;
  // check whether end is reached
  if ( file->fpos >= file->fsize ) {
    // clear block data if set
    if ( file->block.data ) {
      free( file->block.data );
      // reset sector to 0 and data
      file->block.data = NULL;
      file->block.sector = 0;
    }
    // success
    return EOK;
  }
  // free up set block
  if ( file->block.data ) {
    // free up block data
    free( file->block.data );
    // reset sector to 0 and data
    file->block.data = NULL;
    file->block.sector = 0;
  }
  // calculate current block
  uint64_t block_size = fs->bdev->bdif->block_size;
  uint64_t current_block = file->fpos / block_size;
  // get next block index
  if ( 0 == file->cluster ) {
    // next root block is root directory offset + sector
    uint64_t rootdir_offset;
    uint64_t rootdir_size;
    fat_directory_t dir;
    memset( &dir, 0, sizeof( dir ) );
    dir.file.cluster = file->cluster;
    dir.file.mp = file->mp;
    // get offset and size of root directory
    int result = fat_rootdir_offset_size( &dir, &rootdir_offset, &rootdir_size );
    if ( EOK != result ) {
      return result;
    }
    // allocate buffer if not allocated
    if ( ! file->block.data ) {
      // allocate block
      file->block.data = malloc( ( size_t )block_size );
      if ( ! file->block.data ) {
        return ENOMEM;
      }
      // clear out block
      memset( file->block.data, 0, ( size_t )block_size );
    }
    // read bytes from device
    result = common_blockdev_bytes_read(
      bdev,
      ( rootdir_offset + current_block ) * block_size,
      file->block.data,
      block_size
    );
    // handle error
    if ( EOK != result ) {
      return result;
    }
  } else {
    // allocate buffer if not allocated
    if ( ! file->block.data ) {
      // allocate block
      file->block.data = malloc( ( size_t )block_size );
      if ( ! file->block.data ) {
        return ENOMEM;
      }
      // clear out block
      memset( file->block.data, 0, ( size_t )block_size );
    }
    uint64_t lba;
    // transform data cluster to lba
    int result = fat_cluster_to_lba( fs, file->cluster, &lba );
    if ( EOK != result ) {
      return result;
    }
    // read bytes from device
    result = common_blockdev_bytes_read(
      bdev,
      lba * block_size,
      file->block.data,
      block_size
    );
    // handle error
    if ( EOK != result ) {
      return result;
    }
  }
  // return success
  return EOK;
}
