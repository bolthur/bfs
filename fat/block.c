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
#include <common/blockdev.h>
#include <common/mountpoint.h>
#include <fat/type.h>
#include <fat/fs.h>
#include <fat/block.h>
#include <fat/cluster.h>
#include <fat/rootdir.h>
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
 * @brief Helper to load directory blocks
 *
 * @param dir
 * @return int
 */
BFSFAT_NO_EXPORT int fat_block_load_directory( fat_directory_t* dir ) {
  if ( ! dir ) {
    return EINVAL;
  }
  // extract fs pointer
  fat_fs_t* fs = dir->file.mp->fs;
  // load chain
  int result = fat_cluster_load( fs, &dir->file );
  if ( EOK != result ) {
    return result;
  }

  // load root directory
  if ( 0 == dir->file.cluster ) {
    // next root block is root directory offset + sector
    uint64_t rootdir_offset;
    uint64_t rootdir_size;
    // get offset and size of root directory
    result = fat_rootdir_offset_size( dir, &rootdir_offset, &rootdir_size );
    if ( EOK != result ) {
      return result;
    }
    dir->block_count = rootdir_size;
    dir->blocks = malloc( ( size_t )dir->block_count * sizeof( fat_block_t ) );
    if ( ! dir->blocks ) {
      return ENOMEM;
    }
    memset( dir->blocks, 0, ( size_t )dir->block_count * sizeof( fat_block_t ) );
    for ( uint64_t i = 0; i < dir->block_count; i++ ) {
      // allocate data block
      uint8_t* data = malloc( fs->bdev->bdif->block_size );
      if ( ! data ) {
        for ( uint64_t j = 0; j < dir->block_count; j++ ) {
          if ( dir->blocks[ j ].data ) {
            free( dir->blocks[ j ].data );
          }
        }
        free( dir->blocks );
        dir->blocks = NULL;
        return ENOMEM;
      }
      dir->blocks[ i ].data = data;
      dir->blocks[ i ].data_size = fs->bdev->bdif->block_size;
      // clear out block
      memset( dir->blocks[ i ].data, 0, ( size_t )fs->bdev->bdif->block_size );
      // read bytes from device
      result = common_blockdev_bytes_read(
        fs->bdev,
        ( rootdir_offset + i ) * fs->bdev->bdif->block_size,
        dir->blocks[ i ].data,
        fs->bdev->bdif->block_size
      );
      // handle error
      if ( EOK != result ) {
        for ( uint64_t j = 0; j < dir->block_count; j++ ) {
          if ( dir->blocks[ j ].data ) {
            free( dir->blocks[ j ].data );
          }
        }
        free( dir->blocks );
        dir->blocks = NULL;
        return result;
      }
    }
  } else {
    uint64_t fpos = dir->file.fpos;
    // cache block size
    uint64_t cluster_size = fs->superblock.sectors_per_cluster
      * fs->superblock.bytes_per_sector;
    // different cluster size for root directory for non fat32
    if ( dir->file.cluster == 0 && FAT_FAT32 != fs->type ) {
      cluster_size = fs->superblock.bytes_per_sector;
    }
    dir->block_count = 0;
    // load cluster by cluster
    for ( uint64_t i = 0; i < dir->file.chain_size; i++ ) {
      // unload first
      result = fat_block_unload( &dir->file );
      if ( EOK != result ) {
        return result;
      }
      dir->file.fpos = i * cluster_size;
      // load block
      result = fat_block_load( &dir->file, cluster_size );
      // handle error
      if ( EOK != result ) {
        return result;
      }
      if ( ! dir->file.block.data ) {
        return EINVAL;
      }
      // extend blocks
      fat_block_t* block;
      if ( ! dir->blocks ) {
        block = malloc( sizeof( fat_block_t ) );
      } else {
        block = realloc( dir->blocks, ( ( size_t )dir->block_count + 1 ) * sizeof( fat_block_t ) );
      }
      // handle error
      if ( ! block ) {
        return ENOMEM;
      }
      memset( &block[ i ], 0, sizeof( fat_block_t ) );
      // get last index
      uint8_t* data = malloc( dir->file.block.data_size );
      if ( ! data ) {
        return ENOMEM;
      }
      block[ i ].data = data;
      block[ i ].data_size = dir->file.block.data_size;
      // copy over data
      memcpy(
        block[ i ].data,
        dir->file.block.data,
        dir->file.block.data_size
      );
      // increment block count
      dir->blocks = block;
      dir->block_count++;
    }
    dir->file.fpos = fpos;
  }
  // return success
  return EOK;
}

/**
 * @brief Unload directory blocks
 *
 * @param dir
 * @return int
 */
BFSFAT_NO_EXPORT int fat_block_unload_directory( fat_directory_t* dir ) {
  if ( ! dir ) {
    return EINVAL;
  }
  for ( uint64_t i = 0; i < dir->block_count; i++ ) {
    if ( dir->blocks[ i ].data ) {
      free( dir->blocks[ i ].data );
    }
  }
  free( dir->blocks );
  dir->blocks = NULL;
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
    file->block.sector = ( rootdir_offset + current_block );
    file->block.cluster = 0;
    file->block.block = current_block;
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
    // adjust current block
    current_block = file->fpos / size;
    // get cluster by number
    uint64_t cluster = file->chain[ current_block ];
    // transform data cluster to lba
    uint64_t lba;
    result = fat_cluster_to_lba( fs, cluster, &lba );
    if ( EOK != result ) {
      return result;
    }
    // read bytes from device
    result = common_blockdev_bytes_read(
      bdev,
      lba * block_size,
      file->block.data,
      size
    );
    // handle error
    if ( EOK != result ) {
      return result;
    }
    file->block.sector = lba;
    file->block.cluster = cluster;
    file->block.block = current_block;
  }
  // return success
  return EOK;
}

/**
 * @brief Method to load a fat block by file offset
 *
 * @param file file to write block data to
 * @param size size to write
 * @return int
 */
BFSFAT_NO_EXPORT int fat_block_write( fat_file_t* file, uint64_t size ) {
  if ( ! file || !file->block.data ) {
    return EINVAL;
  }
  common_mountpoint_t* mp = file->mp;
  fat_fs_t* fs = mp->fs;
  // translate to lba
  uint64_t lba = file->block.sector;
  uint64_t block_size = fs->bdev->bdif->block_size;
  if ( 0 != file->cluster ) {
    block_size = size;
  }
  // write cluster
  return common_blockdev_bytes_write(
    fs->bdev,
    lba * fs->bdev->bdif->block_size,
    file->block.data,
    block_size
  );
}
