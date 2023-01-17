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
#include <unistd.h>
#include <fcntl.h>
#include <common/stdio.h> // IWYU pragma: keep
#include <common/errno.h>
#include <common/file.h>
#include <fat/cluster.h>
#include <fat/file.h>
#include <fat/type.h>
#include <fat/iterator.h>
#include <fat/block.h>
#include <fat/directory.h>
#include <fat/bfsfat_export.h>

/**
 * @brief Fat file seek
 *
 * @param file
 * @param offset
 * @param whence
 * @return int
 */
BFSFAT_EXPORT int fat_file_seek( fat_file_t* file, int64_t offset, uint32_t whence ) {
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
    uint64_t cluster;
    result = fat_cluster_get_free( fs, &cluster );
    if ( EOK != result ) {
      free( dentry );
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
    // mark cluster as used
    result = fat_cluster_set_cluster( fs, cluster, FAT_CLUSTER_EOF );
    if ( EOK != result ) {
      free( dentry );
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
    result = fat_directory_dentry_insert( dir, base, false );
    if ( EOK != result ) {
      fat_cluster_set_cluster( fs, cluster, FAT_CLUSTER_UNUSED );
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
    if ( ENOENT == result ) {
      fat_directory_close( dir );
    }
    free( dentry );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // handle truncate
  if ( flags & O_TRUNC ) {
    // try to truncate file to size 0
    result = fat_file_truncate( file, 0 );
    if ( EOK != result ) {
      free( dentry );
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
    // reload entry
    result = fat_directory_entry_by_name( dir, base );
    if ( EOK != result ) {
      free( dentry );
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
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
  file->dentry_pos = dir->file.fpos;
  // handle append mode
  if ( file->flags & O_APPEND ) {
    file->fpos = file->fsize;
  }
  // free up memory
  free( pathdup_base );
  free( pathdup_dir );
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
 *
 * @todo evaluate flags
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
 *
 * @todo add support for unused flags
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
  if ( file->dir ) {
    result = fat_directory_close( file->dir );
    if ( EOK != result ) {
      return result;
    }
    free( file->dir );
    file->dir = NULL;
  }
  if ( file->dentry ) {
    free( file->dentry );
    file->dentry = NULL;
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
  // cache mountpoint and fs
  common_mountpoint_t*mp = file->mp;
  fat_fs_t* fs = mp->fs;
  // ensure that fs is valid
  if ( ! fs ) {
    return EINVAL;
  }
  // cap read size to maximum if exceeding
  if ( file->fpos + size > file->fsize ) {
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
 *
 * @todo add test for this function
 */
BFSFAT_EXPORT int fat_file_truncate( fat_file_t* file, uint64_t size ) {
  if ( ! file || ! file->mp || ! file->dir || ! file->dentry || ! file->cluster ) {
    return EINVAL;
  }
  // handle invalid flags
  if ( !( file->flags & O_RDWR ) && ! ( file->flags & O_RDWR ) ) {
    return EINVAL;
  }
  // cache mountpoint and fs
  common_mountpoint_t*mp = file->mp;
  fat_fs_t* fs = mp->fs;
  // ensure that fs is valid
  if ( ! fs ) {
    return EINVAL;
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
  int result = fat_cluster_get_chain_end_value( fs, &value );
  if ( EOK != result ) {
    return result;
  }
  // handle shrink
  if ( old_count > new_count ) {
    // allocate space for cluster list
    uint64_t* cluster_list = malloc( sizeof( uint64_t ) * old_count );
    if ( ! cluster_list ) {
      return ENOMEM;
    }
    memset( cluster_list, 0, sizeof( uint64_t ) * old_count );
    // load cluster list
    for ( uint64_t index = 0; index < old_count; index++ ) {
      // get cluster
      uint64_t cluster;
      result = fat_cluster_get_by_num( fs, file->cluster, index, &cluster );
      if ( EOK != result ) {
        free( cluster_list );
        return result;
      }
      // push to array
      cluster_list[ index ] = cluster;
    }
    // set last new cluster as chain end
    result = fat_cluster_set_cluster( fs, cluster_list[ new_count - 1 ], value );
    if ( EOK != result ) {
      free( cluster_list );
      return result;
    }
    // mark other clusters as unused
    for ( uint64_t index = new_count; index < old_count; index++ ) {
      result = fat_cluster_set_cluster(
        fs,
        cluster_list[ index ],
        FAT_CLUSTER_UNUSED
      );
      if ( EOK != result ) {
        free( cluster_list );
        return result;
      }
    }
    // free cluster
    free( cluster_list );
  // handle extend
  } else if ( old_count < new_count ) {
    uint64_t block_count = new_count - old_count;
    for ( uint64_t index = 0; index < block_count; index++ ) {
      // extend
      result = fat_file_extend_cluster( file, 1 );
      if ( EOK != result ) {
        return result;
      }
      file->fsize = ( old_count + index + 1 ) * cluster_size;
      // adjust position to last cluster
      uint64_t fpos = file->fpos;
      file->fpos = ( old_count + index ) * cluster_size;
      // load block
      result = fat_block_load( file, cluster_size );
      if ( EOK != result ) {
        return result;
      }
      // handle nothing loaded
      if ( ! file->block.data ) {
        return EIO;
      }
      // clear out
      memset( file->block.data, 0, cluster_size );
      // write back
      result = fat_block_write( file, cluster_size );
      if ( EOK != result ) {
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
      file->fpos = fpos;
      return result;
    }
    // handle nothing loaded
    if ( ! file->block.data ) {
      file->fpos = fpos;
      return EIO;
    }
    // set new space to 0
    memset( file->block.data + file->fsize, 0, size - file->fsize );
    // write back
    result = fat_block_write( file, cluster_size );
    if ( EOK != result ) {
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
    return result;
  }
  // return success
  return EOK;
}

/**
 * @brief Write buffer to file
 *
 * @param file
 * @param buffer
 * @param size
 * @param write_count
 * @return int
 *
 * @todo implement function
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
      file->fpos = fpos;
      return result;
    }
    // extend size to new size
    // FIXME: CHECK WHETHER SIZE EXTEND HERE IS NECESSARY
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
      file->fpos = fpos;
      return result;
    }
    // overwrite block data
    memcpy(
      file->block.data + copy_offset,
      ( uint8_t* )buffer + copy_count,
      copy_size
    );
    // write back block
    result = fat_block_write( file, cluster_size );
    if ( EOK != result ) {
      file->fpos = fpos;
      return result;
    }
    // increase copy count
    copy_count += copy_size;
  }

  // update write count
  *write_count = copy_count;
  // restore file position and update fsize
  file->fpos = fpos;
  file->fsize = old_size + copy_count;
  // return success
  return EOK;
}

/**
 * @brief Remove a file
 *
 * @param path
 * @return int
 *
 * @todo implement function
 */
BFSFAT_EXPORT int fat_file_remove( const char* path ) {
  ( void )path;
  return ENOSYS;
}

/**
 * @brief Rename a file
 *
 * @param old_path
 * @param new_path
 * @return int
 *
 * @todo implement function
 */
BFSFAT_EXPORT int fat_file_move( const char* old_path, const char* new_path ) {
  ( void )old_path;
  ( void )new_path;
  return ENOSYS;
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
  // loop and add clusters
  for ( uint64_t index = 0; index < num; index++ ) {
    // find a free cluster
    uint64_t new_cluster;
    int result = fat_cluster_get_free( fs, &new_cluster );
    // handle error
    if ( EOK != result ) {
      return result;
    }
    // allocate cluster size
    uint64_t cluster_size = fs->superblock.bytes_per_sector
      * fs->superblock.sectors_per_cluster;
    // get last cluster
    uint64_t last_cluster;
    uint64_t chain_index = file->fsize / cluster_size;
    if ( file->fsize % cluster_size ) {
      chain_index++;
    }
    result = fat_cluster_get_by_num(
      fs, file->cluster, chain_index, &last_cluster
    );
    // handle error
    if ( EOK != result ) {
      return result;
    }
    // determine cluster chain end value
    uint64_t value;
    result = fat_cluster_get_chain_end_value( fs, &value );
    if ( EOK != result ) {
      return result;
    }
    // try to mark new cluster as chain end
    result = fat_cluster_set_cluster( fs, new_cluster, value );
    // handle error
    if ( EOK != result ) {
      return result;
    }
    // set new cluster within block current
    result = fat_cluster_set_cluster( fs, last_cluster, new_cluster );
    // handle error
    if ( EOK != result ) {
      fat_cluster_set_cluster( fs, new_cluster, FAT_CLUSTER_UNUSED );
      return result;
    }
  }
  // return success
  return EOK;
}
