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
#include <common/transaction.h>
#include <ext/directory.h>
#include <ext/file.h>
#include <ext/inode.h>
#include <ext/link.h>
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
    result = common_transaction_begin( fs->bdev );
    if ( EOK != result ) {
      ext_directory_close( dir );
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
    // allocate inode
    ext_structure_inode_t inode;
    uint64_t number;
    memset( &inode, 0, sizeof( inode ) );
    result = ext_inode_allocate( fs, &inode, &number );
    if ( EOK != result ) {
      common_transaction_rollback( fs->bdev );
      ext_directory_close( dir );
      free( pathdup_base );
      free( pathdup_dir );
      free( dir );
      return result;
    }
    // prepare inode
    inode.i_mode = EXT_INODE_EXT2_S_IFREG;
    inode.i_dtime = 0;
    // link in parent directory
    result = ext_link_link( fs, dir, &inode, number, base );
    if ( EOK != result ) {
      common_transaction_rollback( fs->bdev );
      ext_directory_close( dir );
      free( pathdup_base );
      free( pathdup_dir );
      return result;
    }
    result = common_transaction_commit( fs->bdev );
    if ( EOK != result ) {
      ext_directory_close( dir );
      free( pathdup_base );
      free( pathdup_dir );
      return result;
    }
    // try to get entry again
    result = ext_directory_entry_by_name( dir, base );
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

/**
 * @brief Remove a file
 *
 * @param path
 * @return int
 */
BFSEXT_EXPORT int ext_file_remove( const char* path ) {
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
  ext_fs_t* fs = mp->fs;
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
  ext_directory_t* dir = malloc( sizeof( *dir ) );
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
  result = ext_directory_open( dir, dirpath );
  if ( EOK != result ) {
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // unlink
  result = ext_link_unlink( fs, dir, base );
  if ( EOK != result ) {
    ext_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // close directory and free memory
  result = ext_directory_close( dir );
  if ( EOK != result ) {
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // free memory
  free( dir );
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
 * @brief Move file
 *
 * @param old_path
 * @param new_path
 * @return int
 */
BFSEXT_EXPORT int ext_file_move( const char* old_path, const char* new_path ) {
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
  ext_fs_t* fs = mp->fs;
  // handle read only
  if ( fs->read_only ) {
    return EROFS;
  }
  // start transaction
  int result = common_transaction_begin( fs->bdev );
  if ( EOK != result ) {
    return result;
  }
  // open destination to check whether it exists
  ext_file_t file;
  ext_directory_t source, target;
  memset( &source, 0, sizeof( source ) );
  memset( &target, 0, sizeof( target ) );
  memset( &file, 0, sizeof( file ) );
  result = ext_file_open2( &file, new_path, O_RDONLY );
  if ( ENOENT != result ) {
    ext_file_close( &file );
    common_transaction_rollback( fs->bdev );
    return EOK == result ? EEXIST : result;
  }
  // open source to check whether it exists
  result = ext_file_open2( &file, old_path, O_RDONLY );
  if ( EOK != result ) {
    ext_file_close( &file );
    common_transaction_rollback( fs->bdev );
    return EOK == result ? ENOENT : result;
  }
  result = ext_file_close( &file );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // duplicate strings
  char* dir_dup_old_path = strdup( old_path );
  if ( ! dir_dup_old_path ) {
    common_transaction_rollback( fs->bdev );
    return ENOMEM;
  }
  char* base_dup_old_path = strdup( old_path );
  if ( ! base_dup_old_path ) {
    free( dir_dup_old_path );
    common_transaction_rollback( fs->bdev );
    return ENOMEM;
  }
  char* dir_dup_new_path = strdup( new_path );
  if ( ! dir_dup_new_path ) {
    free( base_dup_old_path );
    free( dir_dup_old_path );
    common_transaction_rollback( fs->bdev );
    return ENOMEM;
  }
  char* base_dup_new_path = strdup( new_path );
  if ( ! base_dup_new_path ) {
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    common_transaction_rollback( fs->bdev );
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
    common_transaction_rollback( fs->bdev );
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
    common_transaction_rollback( fs->bdev );
    return ENOTSUP;
  }
  // Add trailing slash if not existing, necessary, when opening root directory
  if ( CONFIG_PATH_SEPARATOR_CHAR != dir_new_path[ strlen( dir_new_path ) - 1 ] ) {
    strcat( dir_new_path, CONFIG_PATH_SEPARATOR_STRING );
  }
  // clear out source and target
  memset( &source, 0, sizeof( source ) );
  memset( &target, 0, sizeof( target ) );
  memset( &file, 0, sizeof( file ) );
  // open source file
  result = ext_file_open2( &file, old_path, O_RDONLY );
  if ( EOK != result ) {
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // open source directory
  result = ext_directory_open( &source, dir_old_path );
  if ( EOK != result ) {
    ext_file_close( &file );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // open target directory
  result = ext_directory_open( &target, dir_new_path );
  if ( EOK != result ) {
    ext_directory_close( &source );
    ext_file_close( &file );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // link in target
  result = ext_link_link(
    target.mp->fs,
    &target,
    &file.inode,
    file.inode_number,
    base_new_path
  );
  if ( EOK != result ) {
    ext_directory_close( &target );
    ext_directory_close( &source );
    ext_file_close( &file );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // unlink in source
  if ( 0 == strcmp( dir_new_path, dir_old_path ) ) {
    result = ext_link_unlink( target.mp->fs, &target, base_old_path );
  } else {
    result = ext_link_unlink( source.mp->fs, &source, base_old_path );
  }
  if ( EOK != result ) {
    ext_directory_close( &target );
    ext_directory_close( &source );
    ext_file_close( &file );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // close source directory
  result = ext_file_close( &file );
  if ( EOK != result ) {
    ext_directory_close( &target );
    ext_directory_close( &source );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // close source directory
  result = ext_directory_close( &source );
  if ( EOK != result ) {
    ext_directory_close( &target );
    ext_directory_close( &source );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // close target directory
  result = ext_directory_close( &target );
  if ( EOK != result ) {
    ext_directory_close( &target );
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // free duplicates
  free( base_dup_new_path );
  free( dir_dup_new_path );
  free( base_dup_old_path );
  free( dir_dup_old_path );
  // return success
  return common_transaction_commit( fs->bdev );
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

/**
 * @brief Truncate file
 *
 * @param file
 * @param size
 * @return int
 *
 * @todo implement
 */
BFSEXT_EXPORT int ext_file_truncate( ext_file_t* file, uint64_t size ) {
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

/**
 * @brief Write file
 *
 * @param file
 * @param buffer
 * @param size
 * @param write_count
 * @return int
 */
BFSEXT_EXPORT int ext_file_write(
  ext_file_t* file,
  void* buffer,
  uint64_t size,
  uint64_t* write_count
) {
  // validate parameter
  if ( ! file || ! file->mp || ! buffer || ! write_count ) {
    return EINVAL;
  }
  common_mountpoint_t* mp = file->mp;
  ext_fs_t* fs = mp->fs;
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
  // write to file
  result = ext_inode_write_data( fs, &file->inode, file->fpos, size, buffer );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // update write count
  *write_count = size;
  // restore file position and update fsize
  file->fpos = fpos;
  file->fsize = file->fsize + size;
  // return success
  return common_transaction_commit( fs->bdev );
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
