// Copyright (C) 2022 - 2024 bolthur project.
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
#include <inttypes.h>
#include <common/errno.h>
#include <common/blockdev.h>
#include <common/mountpoint.h>
#include <common/log.h>
#include <fat/type.h>
#include <fat/fs.h>
#include <fat/block.h>
#include <fat/cluster.h>
#include <fat/rootdir.h>
#include <fat/structure.h>
#include <fat/bfsfat_export.h>

/**
 * @brief Helper to unload block cache of file
 *
 * @param file
 * @return int
 */
BFSFAT_NO_EXPORT int fat_block_unload( fat_file_t* file ) {
  // validate
  if ( ! file ) {
    return EINVAL;
  }
  // clear out
  if ( file->block.data ) {
    free( file->block.data );
    // reset sector to 0 and data
    file->block.data = NULL;
    file->block.sector = 0;
    file->block.cluster = 0;
    file->block.block = 0;
    file->block.data_size = 0;
  }
  return EOK;
}

/**
 * @brief Method to load a fat block by file offset
 *
 * @param file file to load a block from
 * @param size size of data to load except for fixed sized root directories
 * @return int
 */
BFSFAT_NO_EXPORT int fat_block_load( fat_file_t* file, uint64_t size ) {
  if ( ! file ) {
    return EINVAL;
  }
  int result;
  // extract fs pointer
  fat_fs_t* fs = file->mp->fs;
  // extract block device
  common_blockdev_t* bdev = fs->bdev;
  // calculate current block
  uint64_t block_size = fs->bdev->bdif->block_size;
  uint64_t current_block = file->fpos / block_size;
  // cap by size if size is smaller
  if ( size < block_size ) {
    block_size = size;
    current_block = file->fpos / block_size;
  }
  // adjust current block for root cluster
  if (
    0 == file->cluster
    && block_size != fs->superblock.bytes_per_sector
    && current_block > 0
  ) {
    // calculate correct current block for root dir
    current_block = current_block * block_size / fs->superblock.bytes_per_sector;
  }
  // handle block already loaded
  if ( file->block.data && file->block.block == current_block ) {
    return EOK;
  }
  // load chain
  result = fat_cluster_load( fs, file );
  if ( EOK != result ) {
    return result;
  }
  // clear block data if set
  result = fat_block_unload( file );
  if ( EOK != result ) {
    return result;
  }
  // check whether end is reached and return success
  if ( file->fpos > file->fsize ) {
    return EOK;
  }
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
    result = fat_rootdir_offset_size( &dir, &rootdir_offset, &rootdir_size );
    if ( EOK != result ) {
      return result;
    }
    // allocate buffer if not allocated
    if ( ! file->block.data ) {
      // allocate block
      file->block.data = malloc( ( size_t )block_size );
      file->block.data_size = block_size;
      if ( ! file->block.data ) {
        return ENOMEM;
      }
      // clear out block
      memset( file->block.data, 0, ( size_t )block_size );
    }
    common_log( "Loading block %#"PRIx64"\r\n", ( rootdir_offset + current_block ) );
    // read bytes from device
    result = common_blockdev_bytes_read(
      bdev,
      ( rootdir_offset + current_block ) * fs->superblock.bytes_per_sector,
      file->block.data,
      block_size
    );
    // handle error
    if ( EOK != result ) {
      return result;
    }
    file->block.sector = rootdir_offset + current_block;
    file->block.cluster = 0;
    file->block.block = current_block;
    file->block.data_size = block_size;
  } else {
    // allocate buffer if not allocated
    if ( ! file->block.data ) {
      // allocate block
      file->block.data = malloc( ( size_t )size );
      if ( ! file->block.data ) {
        return ENOMEM;
      }
      // clear out block
      memset( file->block.data, 0, ( size_t )size );
      file->block.data_size = size;
    }
    // get cluster by number
    uint64_t cluster = file->chain[ current_block ];
    // transform data cluster to lba
    uint64_t lba;
    result = fat_cluster_to_lba( fs, cluster, &lba );
    if ( EOK != result ) {
      return result;
    }
    common_log( "Loading block %#"PRIx64"\r\n", lba );
    // read bytes from device
    result = common_blockdev_bytes_read(
      bdev,
      lba * fs->superblock.bytes_per_sector,
      file->block.data,
      block_size
    );
    // handle error
    if ( EOK != result ) {
      return result;
    }
    file->block.sector = lba;
    file->block.cluster = cluster;
    file->block.block = current_block;
    file->block.data_size = block_size;
  }
  // return success
  return EOK;
}

/**
 * @brief Method to load a fat block by file offset
 *
 * @param file file to write block data to
 * @return int
 */
BFSFAT_NO_EXPORT int fat_block_write( fat_file_t* file ) {
  if ( ! file || !file->block.data ) {
    return EINVAL;
  }
  common_mountpoint_t* mp = file->mp;
  fat_fs_t* fs = mp->fs;
  // translate to lba
  uint64_t lba = file->block.sector;
  uint64_t block_size = file->block.data_size;
  // write cluster
  return common_blockdev_bytes_write(
    fs->bdev,
    lba * fs->superblock.bytes_per_sector,
    file->block.data,
    block_size
  );
}
