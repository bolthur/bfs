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

#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <common/stdio.h> // IWYU pragma: keep
#include <common/errno.h>
#include <common/file.h>
#include <ext/directory.h>
#include <ext/file.h>
#include <ext/inode.h>
#include <ext/bfsext_export.h>

/**
 * @brief Get a file by path
 *
 * @param file file object to use for open
 * @param path path to open
 * @param flags numeric open flags
 * @return int
 */
BFSEXT_NO_EXPORT int ext_file_get(
  ext_file_t* file,
  const char* path,
  int flags
) {
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
  ext_fs_t* fs = mp->fs;
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
  if (
    ( strlen( "." ) == strlen( dirpath ) && '.' == *dirpath )
    || ( strlen( ".." ) == strlen( dirpath ) && 0 == strcmp( "..", dirpath ) )
  ) {
    free( pathdup_base );
    free( pathdup_dir );
    return ENOTSUP;
  }
  // Add trailing slash if not existing, necessary, when opening root directory
  if ( CONFIG_PATH_SEPARATOR_CHAR != dirpath[ strlen( dirpath ) - 1 ] ) {
    strcat( dirpath, CONFIG_PATH_SEPARATOR_STRING );
  }
  // try to open directory
  ext_directory_t* dir = malloc( sizeof( *dir ) );
  if ( ! dir ) {
    free( pathdup_base );
    free( pathdup_dir );
    return ENOTSUP;
  }
  // clear out
  memset( dir, 0, sizeof( *dir ) );
  // open directory
  int result = ext_directory_open( dir, dirpath );
  if ( EOK != result ) {
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // get entry
  result = ext_directory_entry_by_name( dir, base );
  // handle no directory with creation
  if ( ENOENT == result && ( flags & O_CREAT ) ) {
    /// FIXME: CREATE FILE
    ext_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // handle no file
  if ( EOK != result ) {
    ext_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }

  // read inode
  ext_structure_inode_t inode;
  result = ext_inode_read_inode( fs, dir->entry->inode, &inode );
  if ( EOK != result ) {
    ext_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }

  // copy over necessary information
  file->mp = dir->mp;
  memcpy( &file->inode, &inode, sizeof( inode ) );
  file->dir = dir;
  file->flags = ( uint32_t )flags;
  file->fsize = file->inode.i_size;
  file->inode_number = dir->entry->inode;
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
    result = ext_file_truncate( file, 0 );
    if ( EOK != result ) {
      ext_file_close( file );
      return result;
    }
  }
  // return success
  return EOK;
}

int ext_file_remove( const char* path ) {
  ( void )path;
  return ENOTSUP;
}

int ext_file_move( const char* old_path, const char* new_path ) {
  ( void )old_path;
  ( void )new_path;
  return ENOTSUP;
}

/**
 * @brief Open a file
 *
 * @param file
 * @param path
 * @param flags
 * @return int
 */
BFSEXT_EXPORT int ext_file_open(
  ext_file_t* file,
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
  result = ext_file_get( file, path, open_flags );
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
BFSEXT_EXPORT int ext_file_open2(
  ext_file_t* file,
  const char* path,
  int flags
) {
  // validate parameter
  if ( ! file || ! path ) {
    return EINVAL;
  }
  // get file
  int result = ext_file_get( file, path, flags );
  if ( EOK != result ) {
    return result;
  }
  // return success
  return EOK;
}

/**
 * @brief Close file
 *
 * @param file
 * @return int
 */
BFSEXT_EXPORT int ext_file_close( ext_file_t* file ) {
  if ( ! file ) {
    return EINVAL;
  }
  int result = EOK;
  if ( file->dir ) {
    result = ext_directory_close( file->dir );
    if ( EOK != result ) {
      return result;
    }
    free( file->dir );
    file->dir = NULL;
  }
  // overwrite everything with 0
  memset( file, 0, sizeof( ext_file_t ) );
  // return success
  return result;
}

int ext_file_truncate( ext_file_t* file, uint64_t size ) {
  // validate
  if ( ! file || ! file->mp || ! file->dir || ! file->inode_number ) {
    return EINVAL;
  }
  // handle invalid flags
  if ( !( file->flags & O_RDWR ) && ! ( file->flags & O_WRONLY ) ) {
    return EPERM;
  }
  ( void )file;
  ( void )size;
  return ENOTSUP;
}

/**
 * @brief ext file read
 *
 * @param file
 * @param buffer
 * @param size
 * @param read_count
 * @return int
 */
BFSEXT_EXPORT int ext_file_read(
  ext_file_t* file,
  void* buffer,
  uint64_t size,
  uint64_t* read_count
) {
  // validate parameter
  if ( ! file || ! buffer || ! read_count ) {
    return EINVAL;
  }
  // handle not opened
  if ( ! file->inode_number || ! file->mp ) {
    return EINVAL;
  }
  // handle invalid mode
  if ( file->flags & O_WRONLY ) {
    return EPERM;
  }
  // cache mountpoint and fs
  common_mountpoint_t*mp = file->mp;
  ext_fs_t* fs = mp->fs;
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
  int result = ext_inode_read_data(
    fs, &file->inode, file->fpos, size, buffer );
  if ( EOK != result ) {
    return result;
  }
  // set read count
  *read_count = size;
  // return success
  return EOK;
}

int ext_file_write(
  ext_file_t* file,
  void* buffer,
  uint64_t size,
  uint64_t* write_count
) {
  ( void )file;
  ( void )buffer;
  ( void )size;
  ( void )write_count;
  return ENOTSUP;
}

/**
 * @brief ext file seek
 *
 * @param file
 * @param offset
 * @param whence
 * @return int
 */
BFSEXT_EXPORT int ext_file_seek(
  ext_file_t* file,
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
      if ( 0 > offset || ( uint64_t )offset > file->inode.i_size ) {
        return EINVAL;
      }
      // update file position
      file->fpos = ( uint64_t )offset;
      return EOK;
    case SEEK_CUR:
      // handle invalid offsets
      if (
        ( 0 > offset && ( uint64_t )-offset > file->fpos )
        || ( 0 < offset && ( uint64_t )offset > file->inode.i_size - file->fpos )
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
      file->fpos = file->inode.i_size;
      return EOK;
  }
  // invalid whence
  return EINVAL;
}

/**
 * @brief ext file tell
 *
 * @param file
 * @param offset
 * @return int
 */
BFSEXT_EXPORT int ext_file_tell( ext_file_t* file, uint64_t* offset ) {
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
 * @brief ext file size
 *
 * @param file
 * @param size
 * @return int
 */
BFSEXT_EXPORT int ext_file_size( ext_file_t* file, uint64_t* size ) {
  // validate parameter
  if ( ! file || ! size ) {
    return EINVAL;
  }
  // write size
  *size = file->inode.i_size;
  // return success
  return EOK;
}
