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
 * @param file
 * @param path
 * @return int
 *
 * @todo add file creation support
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
  if ( ( flags & O_CREAT ) && fs->read_only ) {
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
  // try to open directory
  fat_directory_t* dir = malloc( sizeof( *dir ) );
  if ( ! dir ) {
    free( pathdup_base );
    free( pathdup_dir );
    return ENOMEM;
  }
  // clear out
  memset( dir, 0, sizeof( *dir ) );
  // open directory
  int result = fat_directory_open( dir, dirpath );
  if ( EOK != result ) {
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
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
    // mark cluster as used
    result = fat_cluster_set_cluster( fs, cluster, FAT_CLUSTER_EOF );
    if ( EOK != result ) {
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
    result = fat_directory_update( dir, base, false );
    if ( EOK != result ) {
      fat_cluster_set_cluster( fs, cluster, FAT_CLUSTER_UNUSED );
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
  // close directory
  result = fat_directory_close( dir );
  if ( EOK != result ) {
    fat_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // free up memory
  free( pathdup_base );
  free( pathdup_dir );
  free( dir );
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
  // get file
  int result = fat_file_get( file, path, O_RDONLY );
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
  // free up block
  if ( file->block.data ) {
    free( file->block.data );
  }
  // overwrite everything with 0
  memset( file, 0, sizeof( fat_file_t ) );
  // return success
  return EOK;
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
    memcpy(
      u8buffer + *read_count,
      file->block.data,
      ( size_t )( copy_size + copy_start )
    );
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
 * @brief Truncate file content
 *
 * @param file
 * @return int
 */
BFSFAT_EXPORT int fat_file_truncate( fat_file_t* file ) {
  ( void )file;
  return ENOSYS;
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
  ( void )file;
  ( void )buffer;
  ( void )size;
  ( void )write_count;
  return ENOSYS;
}

/**
 * @brief Remove a file
 *
 * @param path
 * @return int
 */
BFSFAT_EXPORT int fat_file_remove( const char* path ) {
  ( void )path;
  return ENOSYS;
}

/**
 * @brief Create a file link
 *
 * @param path
 * @param link_path
 * @return int
 */
BFSFAT_EXPORT int fat_file_link( const char* path, const char* link_path ) {
  ( void )path;
  ( void )link_path;
  return ENOSYS;
}

/**
 * @brief Rename a file
 *
 * @param old_path
 * @param new_path
 * @return int
 */
BFSFAT_EXPORT int fat_file_rename( const char* old_path, const char* new_path ) {
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
  if ( ! file || ! file->mp->fs || ! num ) {
    return EINVAL;
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
    result = fat_cluster_get_by_num(
      fs, file->cluster, file->fsize / cluster_size, &last_cluster
    );
    // handle error
    if ( EOK != result ) {
      return result;
    }
    // determine cluster chain end value
    uint64_t value;
    if ( FAT_FAT12 == fs->type ) {
      value = FAT_FAT12_CLUSTER_CHAIN_END;
    } else if ( FAT_FAT16 == fs->type ) {
      value = FAT_FAT16_CLUSTER_CHAIN_END;
    } else if ( FAT_FAT32 == fs->type ) {
      value = FAT_FAT32_CLUSTER_CHAIN_END;
    } else {
      return EINVAL;
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
    // increase directory file size
    file->fsize += cluster_size;
  }
  // return success
  return EOK;
}

/**
 * @brief Helper to parse open flags
 *
 * @param flags
 * @param file_flags
 * @return int
 */
BFSFAT_NO_EXPORT int fat_file_parse_flags( const char* flags, int *file_flags ) {
  if ( ! flags || ! file_flags ) {
    return EINVAL;
  }
  // handle read
  if ( ! strcmp( flags, "r" ) || ! strcmp( flags, "rb" ) ) {
    *file_flags = O_RDONLY;
    return EOK;
  }
  // handle write
  if ( ! strcmp( flags, "w" ) || ! strcmp( flags, "wb" ) ) {
    *file_flags = O_WRONLY | O_CREAT | O_TRUNC;
    return EOK;
  }
  // handle append
  if ( ! strcmp( flags, "a" ) || ! strcmp( flags, "ab" ) ) {
    *file_flags = O_WRONLY | O_CREAT | O_APPEND;
    return EOK;
  }
  // handle r+
  if (
    ! strcmp( flags, "r+" )
    || ! strcmp( flags, "rb+" )
    || ! strcmp( flags, "r+b" )
  ) {
    *file_flags = O_RDWR;
    return EOK;
  }
  // handle w+
  if (
    ! strcmp( flags, "w+" )
    || ! strcmp( flags, "wb+" )
    || ! strcmp( flags, "w+b" )
  ) {
    *file_flags = O_RDWR | O_CREAT | O_TRUNC;
    return EOK;
  }
  // handle a+
  if (
    ! strcmp( flags, "a+" )
    || ! strcmp( flags, "ab+" )
    || ! strcmp( flags, "a+b" )
  ) {
    *file_flags = O_RDWR | O_CREAT | O_APPEND;
    return EOK;
  }
  // return invalid
  return EINVAL;
}
