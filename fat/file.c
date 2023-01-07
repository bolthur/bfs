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
// IWYU pragma: no_include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <common/stdio.h> // IWYU pragma: keep
#include <common/errno.h>
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
BFSFAT_NO_EXPORT int fat_file_get( fat_file_t* file, const char* path ) {
  // validate parameter
  if ( ! file || ! path ) {
    return EINVAL;
  }
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
  // lock
  COMMON_MP_LOCK( dir->file.mp );
  // allocate iterator
  fat_iterator_directory_t* it = malloc( sizeof( *it ) );
  if ( ! it ) {
    COMMON_MP_UNLOCK( dir->file.mp );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return ENOMEM;
  }
  // clear space
  memset( it, 0, sizeof( *it ) );
  // setup iterator
  result = fat_iterator_directory_init( it, dir, 0 );
  if ( EOK != result ) {
    fat_iterator_directory_fini( it );
    COMMON_MP_UNLOCK( dir->file.mp );
    free( it );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // loop while an entry is existing
  while ( it->entry ) {
    // second case: matching entry
    if ( 0 == strcasecmp( it->data->name, base ) ) {
      break;
    }
    // get next iterator
    result = fat_iterator_directory_next( it );
    if ( EOK != result ) {
      fat_iterator_directory_fini( it );
      COMMON_MP_UNLOCK( dir->file.mp );
      free( it );
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
  }
  // check for no entry
  if (
    ! it->entry
    || ( it->entry->attributes & FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY )
  ) {
    fat_iterator_directory_fini( it );
    COMMON_MP_UNLOCK( dir->file.mp );
    free( it );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return ENOENT;
  }
  // copy over necessary information
  file->mp = dir->file.mp;
  file->cluster = ( ( uint32_t )it->entry->first_cluster_upper << 16 )
    | ( uint32_t )it->entry->first_cluster_lower;
  file->fsize = it->entry->file_size;
  // finish iterator
  result = fat_iterator_directory_fini( it );
  if ( EOK != result ) {
    fat_iterator_directory_fini( it );
    COMMON_MP_UNLOCK( dir->file.mp );
    free( it );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // free up memory
  free( it );
  free( pathdup_base );
  free( pathdup_dir );
  free( dir );
  COMMON_MP_UNLOCK( file->mp );
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
  int result = fat_file_get( file, path );
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
  ( void )flags;
  // validate parameter
  if ( ! file || ! path ) {
    return EINVAL;
  }
  // get file
  int result = fat_file_get( file, path );
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
  // lock
  COMMON_MP_LOCK( mp );
  // cap read size to maximum if exceeding
  if ( file->fpos + size > file->fsize ) {
    size -= ( file->fpos + size - file->fsize );
  }
  // calculate sector to start with
  uint64_t block_size = fs->bdev->bdif->block_size;
  uint8_t* u8buffer = buffer;
  *read_count = 0;
  while ( size > 0 ) {
    uint64_t copy_start = file->fpos % block_size;
    uint64_t copy_size = block_size - copy_start;
    // cap copy size
    if ( copy_size > size )
    {
      copy_size = size;
    }
    // load block
    int result = fat_block_load( file, &file->block );
    if ( EOK != result ) {
      COMMON_MP_UNLOCK( mp );
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
      copy_size + copy_start
    );
    // increment read count
    *read_count += copy_size;
    // decrement size
    size -= copy_size;
    // increment position
    file->fpos += copy_size;
  }
  // lock
  COMMON_MP_UNLOCK( mp );
  // return success
  return EOK;
}
