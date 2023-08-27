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
// IWYU pragma: no_include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include <common/stdio.h> // IWYU pragma: keep
#include <common/errno.h>
#include <common/file.h>
#include <common/mountpoint.h>
#include <common/transaction.h>
#include <fat/fs.h>
#include <fat/structure.h>
#include <fat/cluster.h>
#include <fat/file.h>
#include <fat/type.h>
#include <fat/block.h>
#include <fat/directory.h>
#include <fat/bfsfat_export.h>
#include <bfsconfig.h>

/**
 * @brief Fat file seek
 *
 * @param file
 * @param offset
 * @param whence
 * @return int
 */
BFSFAT_EXPORT int fat_file_seek(
  fat_file_t* file,
  int64_t offset,
  uint32_t whence
) {
  // validate parameter
  if ( ! file ) {
    return EINVAL;
  }
  // handle different whence values
  switch ( whence ) {
    case SEEK_SET:
      // check for negative offset or to big offset
      if ( 0 > offset || ( uint64_t )offset > file->fsize ) {
        return EINVAL;
      }
      // update file position
      file->fpos = ( uint64_t )offset;
      return EOK;
    case SEEK_CUR:
      // handle invalid offsets
      if (
        ( 0 > offset && ( uint64_t )-offset > file->fpos )
        || ( 0 < offset && ( uint64_t )offset > file->fsize - file->fpos )
      ) {
        return EINVAL;
      }
      // update file position
      if ( 0 > offset ) {
        file->fpos -= ( uint64_t )( offset * -1 );
        return EOK;
      }
      file->fpos += ( uint64_t )offset;
      return EOK;
    case SEEK_END:
      // update file position
      file->fpos = file->fsize;
      return EOK;
  }
  // invalid whence
  return EINVAL;
}

/**
 * @brief Get current file position
 *
 * @param file
 * @param offset
 * @return int
 */
BFSFAT_EXPORT int fat_file_tell( fat_file_t* file, uint64_t* offset ) {
  // validate parameter
  if ( ! file || ! offset ) {
    return EINVAL;
  }
  // set offset
  *offset = file->fpos;
  // return success
  return EOK;
}

/**
 * @brief Get file size
 *
 * @param file
 * @param size
 * @return int
 */
BFSFAT_EXPORT int fat_file_size( fat_file_t* file, uint64_t* size ) {
  // validate parameter
  if ( ! file || ! size ) {
    return EINVAL;
  }
  // write size
  *size = file->fsize;
  // return success
  return EOK;
}

/**
 * @brief Get a file by path
 *
 * @param file file object to use for open
 * @param path path to open
 * @param flags numeric open flags
 * @return int
 */
BFSFAT_NO_EXPORT int fat_file_get( fat_file_t* file, const char* path, int flags ) {
  // validate parameter
  if ( ! file || ! path ) {
    return EINVAL;
  }
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_find( path );
  if ( ! mp ) {
    return ENOMEM;
  }
  // get fs
  fat_fs_t* fs = mp->fs;
  // handle create flag with read only
  if (
    (
      ( flags & O_CREAT )
      || ( flags & O_WRONLY )
      || ( flags & O_RDWR )
      || ( flags & O_TRUNC )
    ) && fs->read_only
  ) {
    return EROFS;
  }
  // duplicate path for base and dirname
  char* pathdup_base = strdup( path );
  if ( ! pathdup_base ) {
    return ENOMEM;
  }
  char* pathdup_dir = strdup( path );
  if ( ! pathdup_dir ) {
    free( pathdup_base );
    return ENOMEM;
  }
  // extract base and dirname
  char* base = basename( pathdup_base );
  char* dirpath  = dirname( pathdup_dir );
  // check for unsupported
  if ( '.' == *dirpath ) {
    free( pathdup_base );
    free( pathdup_dir );
    return ENOTSUP;
  }
  // Add trailing slash if not existing, necessary, when opening root directory
  if ( CONFIG_PATH_SEPARATOR_CHAR != dirpath[ strlen( dirpath ) - 1 ] ) {
    strcat( dirpath, CONFIG_PATH_SEPARATOR_STRING );
  }
  // allocate directory entry
  fat_structure_directory_entry_t* dentry = malloc( sizeof( *dentry ) );
  if ( ! dentry ) {
    free( pathdup_base );
    free( pathdup_dir );
    return ENOTSUP;
  }
  memset( dentry, 0, sizeof( *dentry ) );
  // try to open directory
  fat_directory_t* dir = malloc( sizeof( *dir ) );
  if ( ! dir ) {
    free( dentry );
    free( pathdup_base );
    free( pathdup_dir );
    return ENOMEM;
  }
  // clear out
  memset( dir, 0, sizeof( *dir ) );
  // open directory
  int result = fat_directory_open( dir, dirpath );
  if ( EOK != result ) {
    free( dentry );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // get entry
  result = fat_directory_entry_by_name( dir, base );
  // handle no directory with creation
  if ( ENOENT == result && ( flags & O_CREAT ) ) {
    // start transaction
    result = common_transaction_begin( fs->bdev );
    if ( EOK != result ) {
      fat_directory_close( dir );
      free( dentry );
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
    // allocate new cluster
    uint64_t free_cluster;
    result = fat_cluster_get_free( fs, &free_cluster );
    if ( EOK != result ) {
      common_transaction_rollback( fs->bdev );
      fat_directory_close( dir );
      free( dentry );
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
    result = fat_cluster_set_cluster( fs, free_cluster, FAT_CLUSTER_EOF );
    if ( EOK != result ) {
      common_transaction_rollback( fs->bdev );
      fat_directory_close( dir );
      free( dentry );
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
    // try to extend directory
    result = fat_directory_dentry_insert( dir, base, free_cluster, false );
    if ( EOK != result ) {
      common_transaction_rollback( fs->bdev );
      fat_directory_close( dir );
      free( dentry );
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
    result = common_transaction_commit( fs->bdev );
    if ( EOK != result ) {
      common_transaction_rollback( fs->bdev );
      fat_directory_close( dir );
      free( dentry );
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
    // try to get entry again
    result = fat_directory_entry_by_name( dir, base );
  }
  // handle no file
  if ( EOK != result ) {
    fat_directory_close( dir );
    free( dentry );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // copy over necessary information
  file->mp = dir->file.mp;
  file->cluster = ( uint32_t )dir->entry->first_cluster_lower;
  if ( FAT_FAT32 == fs->type ) {
    file->cluster |= ( ( uint32_t )dir->entry->first_cluster_upper << 16 );
  }
  file->fsize = dir->entry->file_size;
  file->dir = dir;
  file->flags = ( uint32_t )flags;
  // copy over directory entry data
  memcpy( dentry, dir->entry, sizeof( *dentry ) );
  file->dentry = dentry;
  file->dentry_pos = dir->entry_pos;
  // handle append mode
  if ( file->flags & O_APPEND ) {
    file->fpos = file->fsize;
  }
  // free up memory
  free( pathdup_base );
  free( pathdup_dir );
  // handle truncate
  if ( flags & O_TRUNC ) {
    // try to truncate file to size 0
    result = fat_file_truncate( file, 0 );
    if ( EOK != result ) {
      fat_file_close( file );
      return result;
    }
  }
  // return success
  return EOK;
}

/**
 * @brief Open a file
 *
 * @param file
 * @param path
 * @param flags
 * @return int
 */
BFSFAT_EXPORT int fat_file_open(
  fat_file_t* file,
  const char* path,
  const char* flags
) {
  // validate parameter
  if ( ! file || ! path || ! flags ) {
    return EINVAL;
  }
  // parse flags
  int open_flags;
  int result = common_file_parse_flags( flags, &open_flags );
  if ( EOK != result ) {
    return result;
  }
  // get file
  result = fat_file_get( file, path, open_flags );
  if ( EOK != result ) {
    return result;
  }
  // return success
  return EOK;
}

/**
 * @brief Open a file
 *
 * @param file
 * @param path
 * @param flags
 * @return int
 */
BFSFAT_EXPORT int fat_file_open2(
  fat_file_t* file,
  const char* path,
  int flags
) {
  // validate parameter
  if ( ! file || ! path ) {
    return EINVAL;
  }
  // get file
  int result = fat_file_get( file, path, flags );
  if ( EOK != result ) {
    return result;
  }
  // return success
  return EOK;
}

/**
 * @brief Closes an opened file
 *
 * @param file
 * @return int
 */
BFSFAT_EXPORT int fat_file_close( fat_file_t* file ) {
  if ( ! file ) {
    return EINVAL;
  }
  int result = EOK;
  // free up block
  if ( file->block.data ) {
    free( file->block.data );
    file->block.data = NULL;
  }
  if ( file->dentry ) {
    free( file->dentry );
    file->dentry = NULL;
  }
  if ( file->chain ) {
    free( file->chain );
    file->chain = NULL;
    file->chain_size = 0;
  }
  if ( file->dir ) {
    result = fat_directory_close( file->dir );
    if ( EOK != result ) {
      return result;
    }
    free( file->dir );
    file->dir = NULL;
  }
  // overwrite everything with 0
  memset( file, 0, sizeof( fat_file_t ) );
  // return success
  return result;
}

/**
 * @brief Read from file to buffer
 *
 * @param file
 * @param buffer
 * @param size
 * @param read_count
 * @return int
 */
int fat_file_read(
  fat_file_t* file,
  void* buffer,
  uint64_t size,
  uint64_t* read_count
) {
  // validate parameter
  if ( ! file || ! buffer || ! read_count ) {
    return EINVAL;
  }
  // handle not opened
  if ( ! file->cluster || ! file->mp ) {
    return EINVAL;
  }
  // handle invalid mode
  if ( file->flags & O_WRONLY ) {
    return EPERM;
  }
  // cache mountpoint and fs
  common_mountpoint_t*mp = file->mp;
  fat_fs_t* fs = mp->fs;
  // ensure that fs is valid
  if ( ! fs ) {
    return EINVAL;
  }
  // cap read size to maximum if exceeding
  if ( 0 == file->fsize ) {
    size = 0;
  } else if ( file->fpos + size > file->fsize ) {
    size -= ( file->fpos + size - file->fsize );
  }
  // calculate sector to start with
  uint64_t cluster_size = fs->superblock.sectors_per_cluster
    * fs->superblock.bytes_per_sector;
  uint8_t* u8buffer = buffer;
  *read_count = 0;
  while ( size > 0 ) {
    uint64_t copy_start = file->fpos % cluster_size;
    uint64_t copy_size = cluster_size - copy_start;
    // cap copy size
    if ( copy_size > size ) {
      copy_size = size;
    }
    // load block
    int result = fat_block_load( file, cluster_size );
    if ( EOK != result ) {
      return result;
    }
    // handle nothing loaded
    if ( ! file->block.data ) {
      break;
    }
    // copy over content
    size_t copy_amount = ( size_t )( copy_size + copy_start );
    uint8_t* u8buffer_start = u8buffer + *read_count;
    memcpy( u8buffer_start, file->block.data, copy_amount );
    // increment read count
    *read_count += copy_size;
    // decrement size
    size -= copy_size;
    // increment position
    file->fpos += copy_size;
  }
  // return success
  return EOK;
}

/**
 * @brief Truncate file to size
 *
 * @param file
 * @param size
 * @return int
 */
BFSFAT_EXPORT int fat_file_truncate( fat_file_t* file, uint64_t size ) {
  if ( ! file || ! file->mp || ! file->dir || ! file->dentry || ! file->cluster ) {
    return EINVAL;
  }
  // handle invalid flags
  if ( !( file->flags & O_RDWR ) && ! ( file->flags & O_WRONLY ) ) {
    return EPERM;
  }
  // cache mountpoint and fs
  common_mountpoint_t*mp = file->mp;
  fat_fs_t* fs = mp->fs;
  // ensure that fs is valid
  if ( ! fs ) {
    return EINVAL;
  }
  int result = common_transaction_begin( fs->bdev );
  if ( EOK != result ) {
    return result;
  }
  // get cluster size
  uint64_t cluster_size = fs->superblock.sectors_per_cluster
    * fs->superblock.bytes_per_sector;
  // calculate new count
  uint64_t new_count = size / cluster_size;
  if ( size % cluster_size ) {
    new_count++;
  }
  // calculate old count
  uint64_t old_count = file->fsize / cluster_size;
  if ( file->fsize % cluster_size ) {
    old_count++;
  }
  // get custer chain end value by type
  uint64_t value;
  result = fat_cluster_get_chain_end_value( fs, &value );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // load chain if not loaded
  result = fat_cluster_load( fs, file );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // handle shrink
  if ( old_count > new_count ) {
    // set last new cluster as chain end if new count is not equal to zero
    if ( 0 != new_count ) {
      result = fat_cluster_set_cluster( fs, file->chain[ new_count - 1 ], value );
      if ( EOK != result ) {
        common_transaction_rollback( fs->bdev );
        return result;
      }
    }
    // mark other clusters as unused
    for ( uint64_t index = new_count; index < old_count; index++ ) {
      result = fat_cluster_set_cluster(
        fs, file->chain[ index ], FAT_CLUSTER_UNUSED
      );
      if ( EOK != result ) {
        common_transaction_rollback( fs->bdev );
        return result;
      }
    }
    // reset first cluster if new count is 0
    if ( 0 == new_count ) {
      file->dentry->first_cluster_lower = 0;
      if ( FAT_FAT32 == fs->type ) {
        file->dentry->first_cluster_upper = 0;
      }
      file->cluster = 0;
      if ( file->chain ) {
        free( file->chain );
        file->chain = NULL;
        file->chain_size = 0;
      }
    } else {
      uint64_t* tmp = realloc( file->chain, ( size_t )new_count * sizeof( uint64_t ) );
      if ( ! tmp ) {
        common_transaction_rollback( fs->bdev );
        return ENOMEM;
      }
      file->chain = tmp;
      file->chain_size = new_count;
    }
  // handle extend
  } else if ( old_count < new_count ) {
    uint64_t block_count = new_count - old_count;
    for ( uint64_t index = 0; index < block_count; index++ ) {
      // extend
      result = fat_file_extend_cluster( file, 1 );
      if ( EOK != result ) {
        common_transaction_rollback( fs->bdev );
        return result;
      }
      file->fsize = ( old_count + index + 1 ) * cluster_size;
      // adjust position to last cluster
      uint64_t fpos = file->fpos;
      file->fpos = ( old_count + index ) * cluster_size;
      // load block
      result = fat_block_load( file, cluster_size );
      if ( EOK != result ) {
        common_transaction_rollback( fs->bdev );
        return result;
      }
      // handle nothing loaded
      if ( ! file->block.data ) {
        common_transaction_rollback( fs->bdev );
        return EIO;
      }
      // clear out
      memset( file->block.data, 0, ( size_t )cluster_size );
      // write back
      result = fat_block_write( file, cluster_size );
      if ( EOK != result ) {
        common_transaction_rollback( fs->bdev );
        return result;
      }
      // restore fpos
      file->fpos = fpos;
    }
  } else if ( size > file->fsize ) {
    // adjust position
    uint64_t fpos = file->fpos;
    file->fpos = ( new_count - 1 ) * cluster_size;
    // load cluster
    result = fat_block_load( file, cluster_size );
    if ( EOK != result ) {
      common_transaction_rollback( fs->bdev );
      file->fpos = fpos;
      return result;
    }
    // handle nothing loaded
    if ( ! file->block.data ) {
      file->fpos = fpos;
      return EIO;
    }
    // set new space to 0
    memset( file->block.data + file->fsize, 0, ( size_t )( size - file->fsize ) );
    // write back
    result = fat_block_write( file, cluster_size );
    if ( EOK != result ) {
      common_transaction_rollback( fs->bdev );
      return result;
    }
    // restore position
    file->fpos = fpos;
  }
  // adjust file size after truncation
  file->dentry->file_size = ( uint32_t )size;
  file->fsize = size;
  // update directory entry
  result = fat_directory_dentry_update(
    file->dir,
    file->dentry,
    file->dentry_pos
  );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // return success
  return common_transaction_commit( fs->bdev );
}

/**
 * @brief Write buffer to file
 *
 * @param file
 * @param buffer
 * @param size
 * @param write_count
 * @return int
 */
BFSFAT_EXPORT int fat_file_write(
  fat_file_t* file,
  void* buffer,
  uint64_t size,
  uint64_t* write_count
) {
  // validate parameter
  if ( ! file || ! file->mp || ! buffer || ! write_count ) {
    return EINVAL;
  }
  common_mountpoint_t* mp = file->mp;
  fat_fs_t* fs = mp->fs;
  int result;
  // handle read only
  if ( fs->read_only ) {
    return EROFS;
  }
  // handle invalid mode
  if ( file->flags & O_RDONLY ) {
    return EPERM;
  }
  // size of 0 means success
  if ( ! size ) {
    return EOK;
  }
  // start transaction
  result = common_transaction_begin( fs->bdev );
  if ( EOK != result ) {
    return result;
  }
  // backup file position
  uint64_t fpos = file->fpos;
  // handle append mode
  if ( file->flags & O_APPEND ) {
    file->fpos = file->fsize;
  }
  // get cluster size
  uint64_t cluster_size = fs->superblock.sectors_per_cluster
    * fs->superblock.bytes_per_sector;
  uint64_t new_size = file->fpos + size;
  uint64_t old_size = file->fsize;
  // extend file if necessary
  if ( new_size > file->fsize ) {
    // calculate block count from new size
    uint64_t new_block_count = new_size / cluster_size;
    if ( new_size % cluster_size ) {
      new_block_count++;
    }
    // calculate block count from old size
    uint64_t old_block_count = file->fsize / cluster_size;
    if ( file->fsize % cluster_size ) {
      old_block_count++;
    }
    // calculate difference
    uint64_t block_count = new_block_count - old_block_count;
    // extend file cluster chain
    result = fat_file_extend_cluster( file, block_count );
    if ( EOK != result ) {
      common_transaction_rollback( fs->bdev );
      file->fpos = fpos;
      return result;
    }
    // extend size to new size
    file->fsize += ( new_size - file->fsize );
  }

  uint64_t copy_count = 0;
  // write cluster per cluster
  while ( size > 0 ) {
    // determine copy size
    uint64_t copy_size = cluster_size;
    uint64_t copy_offset = 0;
    // handle somewhere in between
    if ( file->fpos % cluster_size ) {
      copy_offset = file->fpos % cluster_size;
      copy_size -= copy_offset;
    }
    // cap size
    if ( copy_size > size ) {
      copy_size = size;
    }
    // load sector
    result = fat_block_load( file, cluster_size );
    if ( EOK != result ) {
      common_transaction_rollback( fs->bdev );
      file->fpos = fpos;
      return result;
    }
    // overwrite block data
    memcpy(
      file->block.data + copy_offset,
      ( uint8_t* )buffer + copy_count,
      ( size_t )copy_size
    );
    // write back block
    result = fat_block_write( file, cluster_size );
    if ( EOK != result ) {
      common_transaction_rollback( fs->bdev );
      file->fpos = fpos;
      return result;
    }
    // increase copy count
    copy_count += copy_size;
    // decrement size
    size -= copy_size;
    // increment position
    file->fpos += copy_size;
  }
  // update write count
  *write_count = copy_count;
  // restore file position and update fsize
  file->fpos = fpos;
  file->fsize = old_size + copy_count;
  // return success
  return common_transaction_commit( fs->bdev );
}

/**
 * @brief Remove a file
 *
 * @param path
 * @return int
 */
BFSFAT_EXPORT int fat_file_remove( const char* path ) {
  // validate parameter
  if ( ! path ) {
    return EINVAL;
  }
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_find( path );
  if ( ! mp ) {
    return ENOMEM;
  }
  // get fs
  fat_fs_t* fs = mp->fs;
  // handle read only
  if ( fs->read_only ) {
    return EROFS;
  }
  // duplicate path for base and dirname
  char* pathdup_base = strdup( path );
  if ( ! pathdup_base ) {
    return ENOMEM;
  }
  char* pathdup_dir = strdup( path );
  if ( ! pathdup_dir ) {
    free( pathdup_base );
    return ENOMEM;
  }
  // extract base and dirname
  char* base = basename( pathdup_base );
  char* dirpath  = dirname( pathdup_dir );
  // check for unsupported
  if ( '.' == *dirpath ) {
    free( pathdup_base );
    free( pathdup_dir );
    return ENOTSUP;
  }
  // Add trailing slash if not existing, necessary, when opening root directory
  if ( CONFIG_PATH_SEPARATOR_CHAR != dirpath[ strlen( dirpath ) - 1 ] ) {
    strcat( dirpath, CONFIG_PATH_SEPARATOR_STRING );
  }
  // try to open directory
  fat_directory_t* dir = malloc( sizeof( *dir ) );
  if ( ! dir ) {
    free( pathdup_base );
    free( pathdup_dir );
    return ENOMEM;
  }
  // start transaction
  int result = common_transaction_begin( fs->bdev );
  if ( EOK != result ) {
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // clear out
  memset( dir, 0, sizeof( *dir ) );
  // open directory
  result = fat_directory_open( dir, dirpath );
  if ( EOK != result ) {
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // get entry
  result = fat_directory_entry_by_name( dir, base );
  if ( EOK != result ) {
    fat_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // try to remove it from directory
  result = fat_directory_dentry_remove( dir, dir->entry, dir->entry_pos );
  if ( EOK != result ) {
    fat_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // cache start cluster
  uint32_t start_cluster = ( uint32_t )dir->entry->first_cluster_lower;
  if ( FAT_FAT32 == fs->type ) {
    start_cluster |= ( ( uint32_t )dir->entry->first_cluster_upper << 16 );
  }
  // close directory and free memory
  result = fat_directory_close( dir );
  if ( EOK != result ) {
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  free( dir );
  if ( 0 < start_cluster ) {
    // allocate space for cluster list
    uint64_t* cluster_list = NULL;
    // load complete cluster list
    uint64_t max_index = 0;
    while ( true ) {
      // get cluster
      uint64_t cluster;
      result = fat_cluster_get_by_num( fs, start_cluster, max_index + 1, &cluster );
      // handle end reached
      if ( ENXIO == result ) {
        break;
      }
      // handle error
      if ( EOK != result ) {
        free( cluster_list );
        free( pathdup_base );
        free( pathdup_dir );
        common_transaction_rollback( fs->bdev );
        return result;
      }
      uint64_t* new_list = NULL;
      // allocate list
      if ( ! cluster_list ) {
        new_list = malloc( ( size_t )( ( max_index + 1 ) * sizeof( uint64_t ) ) );
      // reallocate list
      } else {
        new_list = realloc( cluster_list,
          ( size_t )( ( max_index + 1 ) * sizeof( uint64_t ) ) );
      }
      // handle error
      if ( ! new_list ) {
        free( cluster_list );
        free( pathdup_base );
        free( pathdup_dir );
        common_transaction_rollback( fs->bdev );
        return ENOMEM;
      }
      // overwrite old list
      cluster_list = new_list;
      // push back item and increment index
      cluster_list[ max_index++ ] = cluster;
    }
    // free cluster list entries
    for ( uint64_t index = 0; index < max_index; index++ ) {
      // try to free cluster
      result = fat_cluster_set_cluster(
        fs, cluster_list[ index ], FAT_CLUSTER_UNUSED
      );
      // handle error
      if ( EOK != result ) {
        free( cluster_list );
        free( pathdup_base );
        free( pathdup_dir );
        common_transaction_rollback( fs->bdev );
        return result;
      }
    }
    if ( cluster_list ) {
      free( cluster_list );
    }
  }
  // free memory
  free( pathdup_base );
  free( pathdup_dir );
  // commit transaction
  result = common_transaction_commit( fs->bdev );
  if ( EOK != result ) {
    return EOK;
  }
  // return success
  return EOK;
}

/**
 * @brief Rename a file
 *
 * @param old_path
 * @param new_path
 * @return int
 *
 * @todo add transaction
 */
BFSFAT_EXPORT int fat_file_move( const char* old_path, const char* new_path ) {
  if ( ! old_path || ! new_path ) {
    return EINVAL;
  }
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_find( old_path );
  if ( ! mp ) {
    return ENOMEM;
  }
  common_mountpoint_t* mp_new = common_mountpoint_find( new_path );
  if ( ! mp_new ) {
    return ENOMEM;
  }
  // get fs
  fat_fs_t* fs = mp->fs;
  // handle read only
  if ( fs->read_only ) {
    return EROFS;
  }
  // open destination to check whether it exists
  fat_file_t file;
  fat_directory_t source, target;
  memset( &source, 0, sizeof( source ) );
  memset( &target, 0, sizeof( target ) );
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, new_path, O_RDONLY );
  if ( ENOENT != result ) {
    fat_file_close( &file );
    return EOK == result ? EEXIST : result;
  }
  // open source to check whether it exists
  result = fat_file_open2( &file, old_path, O_RDONLY );
  if ( EOK != result ) {
    fat_file_close( &file );
    return EOK == result ? ENOENT : result;
  }
  result = fat_file_close( &file );
  if ( EOK != result ) {
    return result;
  }
  // duplicate strings
  char* dir_dup_old_path = strdup( old_path );
  if ( ! dir_dup_old_path ) {
    return ENOMEM;
  }
  char* base_dup_old_path = strdup( old_path );
  if ( ! base_dup_old_path ) {
    free( dir_dup_old_path );
    return ENOMEM;
  }
  char* dir_dup_new_path = strdup( new_path );
  if ( ! dir_dup_new_path ) {
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return ENOMEM;
  }
  char* base_dup_new_path = strdup( new_path );
  if ( ! base_dup_new_path ) {
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return ENOMEM;
  }
  // get base name of both
  char* dir_old_path = dirname( dir_dup_old_path );
  char* base_old_path = basename( base_dup_old_path );
  // check for unsupported
  if ( '.' == *dir_old_path ) {
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return ENOTSUP;
  }
  // Add trailing slash if not existing, necessary, when opening root directory
  if ( CONFIG_PATH_SEPARATOR_CHAR != dir_old_path[ strlen( dir_old_path ) - 1 ] ) {
    strcat( dir_old_path, CONFIG_PATH_SEPARATOR_STRING );
  }
  char* dir_new_path = dirname( dir_dup_new_path );
  char* base_new_path = basename( base_dup_new_path );
  // check for unsupported
  if ( '.' == *dir_old_path ) {
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return ENOTSUP;
  }
  // Add trailing slash if not existing, necessary, when opening root directory
  if ( CONFIG_PATH_SEPARATOR_CHAR != dir_new_path[ strlen( dir_new_path ) - 1 ] ) {
    strcat( dir_new_path, CONFIG_PATH_SEPARATOR_STRING );
  }
  // clear out source and target
  memset( &source, 0, sizeof( source ) );
  memset( &target, 0, sizeof( target ) );
  // open source directory
  result = fat_directory_open( &source, dir_old_path );
  if ( EOK != result ) {
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return result;
  }
  // open target directory
  result = fat_directory_open( &target, dir_new_path );
  if ( EOK != result ) {
    fat_directory_close( &source );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return result;
  }
  // get entry by base name
  result = fat_directory_entry_by_name( &source, base_old_path );
  if ( EOK != result ) {
    fat_directory_close( &target );
    fat_directory_close( &source );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return result;
  }
  // backup dentry
  fat_structure_directory_entry_t* dentry = malloc( sizeof( *dentry ) );
  if ( ! dentry ) {
    fat_directory_close( &target );
    fat_directory_close( &source );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return result;
  }
  memcpy( dentry, source.entry, sizeof( *dentry ) );
  // remove old entry
  result = fat_directory_dentry_remove( &source, source.entry, source.entry_pos );
  if ( EOK != result ) {
    free( dentry );
    fat_directory_close( &target );
    fat_directory_close( &source );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return result;
  }
  // reload cluster
  result = fat_block_load_directory( &source, true );
  if ( EOK != result ) {
    free( dentry );
    fat_directory_close( &target );
    fat_directory_close( &source );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return result;
  }
  result = fat_block_load_directory( &target, true );
  if ( EOK != result ) {
    free( dentry );
    fat_directory_close( &target );
    fat_directory_close( &source );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return result;
  }
  uint64_t cluster = source.entry->first_cluster_lower;
  if ( FAT_FAT32 == fs->type ) {
    cluster |= ( ( uint64_t )source.entry->first_cluster_upper << 16 );
  }
  // insert new entry
  result = fat_directory_dentry_insert( &target, base_new_path, cluster, false );
  if ( EOK != result ) {
    free( dentry );
    fat_directory_close( &target );
    fat_directory_close( &source );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return result;
  }
  // get entry by file
  result = fat_directory_entry_by_name( &target, base_new_path );
  if ( EOK != result ) {
    free( dentry );
    fat_directory_close( &target );
    fat_directory_close( &source );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return result;
  }
  // copy over stuff
  target.entry->creation_date = dentry->creation_date;
  target.entry->creation_time = dentry->creation_time;
  target.entry->creation_time_tenths = dentry->creation_time_tenths;
  target.entry->file_size = dentry->file_size;
  target.entry->last_accessed_date = dentry->last_accessed_date;
  target.entry->last_modification_date = dentry->last_modification_date;
  target.entry->last_modification_time = dentry->last_modification_time;
  target.entry->attributes = dentry->attributes;
  // update again
  result = fat_directory_dentry_update( &target, target.entry, target.entry_pos );
  if ( EOK != result ) {
    free( dentry );
    fat_directory_close( &target );
    fat_directory_close( &source );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return result;
  }
  // free backup again
  free( dentry );
  // close source directory
  result = fat_directory_close( &source );
  if ( EOK != result ) {
    fat_directory_close( &target );
    fat_directory_close( &source );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return result;
  }
  // close target directory
  result = fat_directory_close( &target );
  if ( EOK != result ) {
    fat_directory_close( &target );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return result;
  }
  // free duplicates
  free( base_dup_new_path );
  free( dir_dup_new_path );
  free( base_dup_old_path );
  free( dir_dup_old_path );
  // return success
  return EOK;
}

/**
 * @brief Extend file by new cluster
 *
 * @param file file to extend clusters of
 * @param num amount of clusters to extend
 * @return int
 */
BFSFAT_NO_EXPORT int fat_file_extend_cluster( fat_file_t* file, uint64_t num ) {
  // validate parameter
  if ( ! file || ! file->mp->fs ) {
    return EINVAL;
  }
  // treat 0 as already done
  if ( ! num ) {
    return EOK;
  }
  // cache file system
  fat_fs_t* fs = file->mp->fs;
  // handle create flag with read only
  if ( fs->read_only ) {
    return EROFS;
  }
  // load cluster chain if not loaded
  int result = fat_cluster_load( fs, file );
  if ( EOK != result ) {
    return result;
  }
  // cache start cluster
  bool update_dentry = false;
  uint64_t start_cluster = file->cluster;
  // determine cluster chain end value
  uint64_t chain_end_value;
  result = fat_cluster_get_chain_end_value( fs, &chain_end_value );
  if ( EOK != result ) {
    return result;
  }
  uint64_t fsize = file->fsize;
  // loop and add clusters
  for ( uint64_t index = 0; index < num; index++ ) {
    // find a free cluster
    uint64_t new_cluster;
    result = fat_cluster_get_free( fs, &new_cluster );
    // handle error
    if ( EOK != result ) {
      return result;
    }
    // calculate cluster size
    uint64_t cluster_size = fs->superblock.bytes_per_sector
      * fs->superblock.sectors_per_cluster;
    // get last cluster
    uint64_t last_cluster = 0;
    if ( start_cluster ) {
      last_cluster = file->chain[ file->chain_size - 1 ];
    }
    // try to mark new cluster as chain end
    result = fat_cluster_set_cluster( fs, new_cluster, chain_end_value );
    // handle error
    if ( EOK != result ) {
      return result;
    }
    if ( start_cluster ) {
      // set new cluster within block current
      result = fat_cluster_set_cluster( fs, last_cluster, new_cluster );
      // handle error
      if ( EOK != result ) {
        fat_cluster_set_cluster( fs, new_cluster, FAT_CLUSTER_UNUSED );
        return result;
      }
      // extend cluster chain
      uint64_t* tmp = realloc(
        file->chain, sizeof( uint64_t ) * ( size_t )( file->chain_size + 1 ) );
      if ( ! tmp ) {
        fat_cluster_set_cluster( fs, new_cluster, FAT_CLUSTER_UNUSED );
        return ENOMEM;
      }
      file->chain = tmp;
      tmp[ file->chain_size++ ] = new_cluster;
    } else {
      // set first cluster
      file->dentry->first_cluster_lower = ( uint16_t )new_cluster;
      if ( FAT_FAT32 == fs->type ) {
        file->dentry->first_cluster_upper = ( uint16_t )(
          ( uint32_t )new_cluster >> 16
        );
      }
      start_cluster = new_cluster;
      file->cluster = new_cluster;
      // initialize cluster chain
      file->chain = malloc( sizeof( uint64_t ) );
      if ( ! file->chain ) {
        fat_cluster_set_cluster( fs, new_cluster, FAT_CLUSTER_UNUSED );
        return ENOMEM;
      }
      file->chain_size = 1;
      file->chain[ 0 ] = new_cluster;
      update_dentry = true;
    }
    file->fsize += cluster_size;
  }
  file->fsize = fsize;
  // perform directory entry update
  if ( update_dentry ) {
    result = fat_directory_dentry_update(
      file->dir,
      file->dentry,
      file->dentry_pos
    );
    if ( EOK != result ) {
      return result;
    }
  }
  // return success
  return EOK;
}

/**
 * @brief Get creation time of file
 *
 * @param file
 * @param ctime
 * @return int
 */
BFSFAT_EXPORT int fat_file_ctime( fat_file_t* file, time_t* ctime ) {
  return fat_directory_dentry_ctime( file->dentry, ctime );
}

/**
 * @brief Get modification time of file
 *
 * @param file
 * @param ctime
 * @return int
 */
BFSFAT_EXPORT int fat_file_mtime( fat_file_t* file, time_t* mtime ) {
  return fat_directory_dentry_mtime( file->dentry, mtime );
}

/**
 * @brief Get access time of file
 *
 * @param file
 * @param ctime
 * @return int
 */
BFSFAT_EXPORT int fat_file_atime( fat_file_t* file, time_t* atime ) {
  return fat_directory_dentry_atime( file->dentry, atime );
}
