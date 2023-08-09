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
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <time.h>
#include <common/blockdev.h>
#include <common/mountpoint.h>
#include <common/errno.h>
#include <common/util.h>
#include <common/transaction.h>
#include <fat/directory.h>
#include <fat/type.h>
#include <fat/structure.h>
#include <fat/rootdir.h>
#include <fat/iterator.h>
#include <fat/cluster.h>
#include <fat/block.h>
#include <fat/fs.h>
#include <fat/file.h>
#include <fat/bfsfat_export.h>
#include <bfsconfig.h>

/**
 * @brief Get directory size
 *
 * @param dir
 * @param size
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_size(
  fat_directory_t* dir,
  uint64_t* size
) {
  // validate
  if ( ! dir || ! dir->file.mp->fs || ! size ) {
    return EINVAL;
  }
  // load cluster chain
  int result = fat_cluster_load( dir->file.mp->fs, &dir->file );
  if ( EOK != result ) {
    return result;
  }
  // cache fs structure
  fat_fs_t* fs = dir->file.mp->fs;
  uint64_t sector_count = dir->file.chain_size;
  // return size
  *size = sector_count * fs->superblock.sectors_per_cluster
    * fs->superblock.bytes_per_sector;
  return EOK;
}

/**
 * @brief Remove a directory
 *
 * @param path
 * @return int
 */
BFSFAT_EXPORT int fat_directory_remove( const char* path ) {
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
  // handle create flag with read only
  if ( fs->read_only ) {
    return EROFS;
  }
  int result = common_transaction_begin( fs->bdev );
  if ( EOK != result ) {
    return result;
  }
  // duplicate path for base and dirname
  char* pathdup_base = strdup( path );
  if ( ! pathdup_base ) {
    common_transaction_rollback( fs->bdev );
    return ENOMEM;
  }
  char* pathdup_dir = strdup( path );
  if ( ! pathdup_dir ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    return ENOMEM;
  }
  // extract base and dirname
  char* base = basename( pathdup_base );
  char* dirpath  = dirname( pathdup_dir );
  // check for unsupported
  if ( '.' == *dirpath ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    return ENOTSUP;
  }
  // Add trailing slash if not existing, necessary, when opening root directory
  if ( CONFIG_PATH_SEPARATOR_CHAR != dirpath[ strlen( dirpath ) - 1 ] ) {
    strcat( dirpath, CONFIG_PATH_SEPARATOR_STRING );
  }
  // handle invalid
  if ( '.' == *base ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    return ENOTSUP;
  }
  // try to open directory
  fat_directory_t* dir = malloc( sizeof( *dir ) );
  if ( ! dir ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    return ENOMEM;
  }
  // clear out
  memset( dir, 0, sizeof( *dir ) );
  // first open complete path
  result = fat_directory_open( dir, path );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // ensure it's empty
  if ( 2 != dir->entry_size ) {
    common_transaction_rollback( fs->bdev );
    fat_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return ENOTEMPTY;
  }
  // close directory again
  result = fat_directory_close( dir );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // open parent directory directory
  result = fat_directory_open( dir, dirpath );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // get entry by name
  result = fat_directory_entry_by_name( dir, base );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    fat_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // try to remove it
  result = fat_directory_dentry_remove( dir, dir->entry, dir->entry_pos );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    fat_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // close directory
  result = fat_directory_close( dir );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // free everything up
  free( pathdup_base );
  free( pathdup_dir );
  free( dir );
  // return success
  return common_transaction_commit( fs->bdev );
}

/**
 * @brief Move a directory
 *
 * @param old_path
 * @param new_path
 * @return int
 *
 * @todo add transaction
 */
BFSFAT_EXPORT int fat_directory_move(
  const char* old_path,
  const char* new_path
) {
  // validate parameter
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
  // handle create flag with read only
  if ( fs->read_only ) {
    return EROFS;
  }
  // variables for source and target dir
  fat_directory_t source, target;
  memset( &source, 0, sizeof( source ) );
  memset( &target, 0, sizeof( target ) );
  // open source directory
  int result = fat_directory_open( &source, old_path );
  if ( EOK != result ) {
    return result;
  }
  // check if empty ( 2 entries . and .. are allowed )
  if ( 2 != source.entry_size ) {
    // close directory
    fat_directory_close( &source );
    // return not empty
    return ENOTEMPTY;
  }
  // close directory again
  result = fat_directory_close( &source );
  if ( EOK != result ) {
    return result;
  }
  // try to open target directory
  result = fat_directory_open( &target, new_path );
  if ( ENOENT != result ) {
    if ( EOK == result ) {
      fat_directory_close( &target );
    }
    return EEXIST;
  }
  // close directory again
  result = fat_directory_close( &target );
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
  // handle invalid
  if ( '.' == *base_old_path || '.' == *base_new_path ) {
    free( base_dup_new_path );
    free( dir_dup_new_path );
    free( base_dup_old_path );
    free( dir_dup_old_path );
    return ENOTSUP;
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
  uint64_t cluster = source.entry->first_cluster_lower;
  if ( FAT_FAT32 == fs->type ) {
    cluster |= ( ( uint64_t )source.entry->first_cluster_upper << 16 );
  }
  // insert new entry
  result = fat_directory_dentry_insert( &target, base_new_path, cluster, true );
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
  uint64_t target_cluster = target.file.cluster;
  // reload blocks
  result = fat_block_unload_directory( &target );
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
  result = fat_block_load_directory( &target );
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
  // free duplicates
  free( base_dup_new_path );
  free( dir_dup_new_path );
  free( base_dup_old_path );
  free( dir_dup_old_path );
  // close target directory
  result = fat_directory_close( &target );
  if ( EOK != result ) {
    fat_directory_close( &target );
    return result;
  }
  // open copied folder
  result = fat_directory_open( &target, new_path );
  if ( EOK != result ) {
    return result;
  }
  // get .. of target
  result = fat_directory_entry_by_name( &target, ".." );
  if ( EOK != result ) {
    fat_directory_close( &target );
    return result;
  }
  // set cluster of .. entry
  target.entry->first_cluster_lower = ( uint16_t )target_cluster;
  if ( FAT_FAT32 == fs->type ) {
    target.entry->first_cluster_upper = ( uint16_t )(
      ( uint32_t )target_cluster >> 16
    );
  }
  // write entry
  result = fat_directory_dentry_update( &target, target.entry, target.entry_pos );
  if ( EOK != result ) {
    fat_directory_close( &target );
    return result;
  }
  // close target directory
  result = fat_directory_close( &target );
  if ( EOK != result ) {
    fat_directory_close( &target );
    return result;
  }
  // return success
  return EOK;
}

/**
 * @brief Create a directory
 *
 * @param path
 * @return int
 */
BFSFAT_EXPORT int fat_directory_make( const char* path ) {
  // validate parameter
  if ( ! path ) {
    return EINVAL;
  }
  common_mountpoint_t* mp = common_mountpoint_find( path );
  if ( ! mp ) {
    return ENOMEM;
  }
  fat_fs_t* fs = mp->fs;
  // handle create flag with read only
  if ( fs->read_only ) {
    return EROFS;
  }
  int result = common_transaction_begin( fs->bdev );
  if ( EOK != result ) {
    return result;
  }
  // duplicate path for base
  char* pathdup_base = strdup( path );
  if ( ! pathdup_base ) {
    common_transaction_rollback( fs->bdev );
    return ENOMEM;
  }
  // duplicate path for dir
  char* pathdup_dir = strdup( path );
  if ( ! pathdup_dir ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    return ENOMEM;
  }
  // extract dirname
  char* base = basename( pathdup_base );
  char* dirpath = dirname( pathdup_dir );
  // check for unsupported
  if ( '.' == *dirpath ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    return ENOTSUP;
  }
  // Add trailing slash if not existing, necessary, when opening root directory
  if ( CONFIG_PATH_SEPARATOR_CHAR != dirpath[ strlen( dirpath ) - 1 ] ) {
    strcat( dirpath, CONFIG_PATH_SEPARATOR_STRING );
  }
  // local variable for directory operations
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // try to open basename
  result = fat_directory_open( &dir, dirpath );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  // ensure that the folder doesn't exist
  result = fat_directory_entry_by_name( &dir, base );
  if ( ENOENT != result ) {
    common_transaction_rollback( fs->bdev );
    fat_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return EOK != result ? result : EEXIST;
  }
  // allocate new cluster
  uint64_t free_cluster;
  result = fat_cluster_get_free( fs, &free_cluster );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    fat_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  result = fat_cluster_set_cluster( fs, free_cluster, FAT_CLUSTER_EOF );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    fat_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  // insert directory entry
  result = fat_directory_dentry_insert( &dir, base, free_cluster, true );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    fat_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  uint64_t parent_cluster = dir.file.cluster;
  // close directory
  result = fat_directory_close( &dir );
  if ( EOK != result ) {
    // free up stuff
    free( pathdup_base );
    free( pathdup_dir );
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // free up stuff
  free( pathdup_base );
  free( pathdup_dir );
  // open new directory
  result = fat_directory_open( &dir, path );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // create dot entries
  result = fat_directory_dentry_insert( &dir, ".", dir.file.cluster, true );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    return result;
  }
  result = fat_directory_dentry_insert( &dir, "..", parent_cluster, true );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // close new directory
  result = fat_directory_close( &dir );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    return result;
  }
  // return success
  return common_transaction_commit( fs->bdev );
}

/**
 * @brief Open a directory
 *
 * @param dir
 * @param path
 * @return int
 */
BFSFAT_EXPORT int fat_directory_open( fat_directory_t* dir, const char* path ) {
  // validate parameters
  if ( ! dir || ! path || *path != CONFIG_PATH_SEPARATOR_CHAR ) {
    return EINVAL;
  }
  // get directory size
  uint64_t size;
  // try to get mountpoint by path
  common_mountpoint_t* mp = common_mountpoint_find( path );
  if ( ! mp ) {
    return ENOENT;
  }
  // open root folder
  int result = fat_rootdir_open( mp, dir );
  if ( EOK != result ) {
    return result;
  }
  // handle root directory
  if ( strlen( path ) == strlen( mp->name ) ) {
    return EOK;
  }

  const char* real_path = path + strlen( mp->name ) - 1;
  char* duppath = strdup( real_path );
  if ( ! duppath ) {
    return ENOMEM;
  }
  char* p = duppath;

  // parse sub directories
  p = strtok( p, CONFIG_PATH_SEPARATOR_STRING );
  while ( p != NULL ) {
    // unload blocks
    result = fat_block_unload_directory( dir );
    if ( EOK != result ) {
      // close current directory
      fat_directory_close( dir );
      // free duplicated path
      free( duppath );
      // return result
      return result;
    }
    // load blocks
    result = fat_block_load_directory( dir );
    if ( EOK != result ) {
      // close current directory
      fat_directory_close( dir );
      // free duplicated path
      free( duppath );
      // return result
      return result;
    }
    // search by name
    result = fat_directory_entry_by_name( dir, p );
    if ( EOK != result ) {
      // close current directory
      fat_directory_close( dir );
      // free duplicated path
      free( duppath );
      // return result
      return result;
    }
    // check for no entry
    if (
      ! dir->entry
      || !( dir->entry->attributes & FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY )
    ) {
      // close current directory
      fat_directory_close( dir );
      // free duplicated path
      free( duppath );
      // return result
      return ENOENT;
    }
    fat_fs_t* fs = mp->fs;
    uint32_t found_start_cluster = ( uint32_t )dir->entry->first_cluster_lower;
    if ( FAT_FAT32 == fs->type ) {
      found_start_cluster |= ( ( uint32_t )dir->entry->first_cluster_upper << 16 );
    }
    // we found a matching entry, so close current directory
    fat_directory_close( dir );
    // prepare directory file object
    dir->file.mp = mp;
    dir->file.fpos = 0;
    dir->file.cluster = found_start_cluster;
    // extract offset and size
    result = fat_directory_size( dir, &size );
    if ( EOK != result ) {
      // close current directory
      fat_directory_close( dir );
      // free duplicated path
      free( duppath );
      // return result
      return result;
    }
    // calculate total size
    dir->file.fsize = size;
    // get next one
    p = strtok( NULL, CONFIG_PATH_SEPARATOR_STRING );
  }
  // free iterator and path duplicate
  free( duppath );
  // set offset to 0
  dir->file.fpos = 0;
  // load blocks
  result = fat_block_unload_directory( dir );
  if ( EOK != result ) {
    // close current directory
    fat_directory_close( dir );
    // return result
    return result;
  }
  result = fat_block_load_directory( dir );
  if ( EOK != result ) {
    // close current directory
    fat_directory_close( dir );
    // return result
    return result;
  }
  // rewind
  result = fat_directory_rewind( dir );
  if ( EOK != result ) {
    return result;
  }
  uint64_t entry_size = 0;
  while ( true ) {
    // get next entry
    result = fat_directory_next_entry( dir );
    // handle error
    if ( EOK != result ) {
      fat_directory_close( dir );
      return result;
    // handle end reached
    } else if ( ! dir->data && ! dir->entry ) {
      break;
    }
    // increment entry size
    entry_size++;
  }
  // push entry size
  dir->entry_size = entry_size;
  // return result of open
  return EOK;
}

/**
 * @brief Close a directory
 *
 * @param dir
 * @return int
 */
BFSFAT_EXPORT int fat_directory_close( fat_directory_t* dir ) {
  // validate parameter
  if ( ! dir ) {
    return EINVAL;
  }
  // clear loaded blocks
  int result = fat_block_unload_directory( dir );
  if ( EOK != result ) {
    return result;
  }
  // different handling for root directory
  if ( 0 == dir->file.cluster ) {
    return fat_rootdir_close( dir );
  }
  // free data
  if ( dir->data ) {
    free( dir->data );
  }
  if ( dir->entry ) {
    free( dir->entry );
  }
  // close file
  result = fat_file_close( &dir->file );
  if ( EOK != result ) {
    return result;
  }
  // overwrite everything with 0
  memset( dir, 0, sizeof( *dir ) );
  // return success
  return EOK;
}

/**
 * @brief Get next entry of directory
 *
 * @param dir
 * @return int
 */
BFSFAT_EXPORT int fat_directory_next_entry( fat_directory_t* dir ) {
  // handle end reached
  if ( dir->file.fpos >= dir->file.fsize ) {
    // free up data
    if ( dir->data ) {
      free( dir->data );
      dir->data = NULL;
    }
    // free up entry
    if ( dir->entry ) {
      free( dir->entry );
      dir->entry = NULL;
    }
    // return success
    return EOK;
  }
  // allocate iterator
  fat_iterator_directory_t* it = malloc( sizeof( *it ) );
  if ( ! it ) {
    return ENOMEM;
  }
  // clear out
  memset( it, 0, sizeof( *it ) );
  // setup iterator
  int result = fat_iterator_directory_init( it, dir, dir->file.fpos );
  if ( EOK != result ) {
    free( it );
    return result;
  }
  if ( dir->data && dir->entry ) {
    // get next iterator
    result = fat_iterator_directory_next( it );
    if ( EOK != result ) {
      free( it );
      return result;
    }
  }
  // allocate data
  if ( ! dir->data && it->data ) {
    dir->data = malloc( sizeof( fat_directory_data_t ) );
    if ( ! dir->data ) {
      free( it );
      return ENOMEM;
    }
    memset( dir->data, 0, sizeof( fat_directory_data_t ) );
  }
  // free up data
  if ( dir->data && ! it->data ) {
    free( dir->data );
    dir->data = NULL;
  }
  // clear data
  if ( dir->data && it->data ) {
    memset( dir->data, 0, sizeof( fat_directory_data_t ) );
  }
  // allocate entry
  if ( ! dir->entry && it->entry ) {
    dir->entry = malloc( sizeof( fat_structure_directory_entry_t ) );
    if ( ! dir->entry ) {
      free( it );
      free( dir->data );
      dir->data = NULL;
      return ENOMEM;
    }
    memset( dir->entry, 0, sizeof( fat_structure_directory_entry_t ) );
  }
  // free up data
  if ( dir->entry && ! it->entry ) {
    free( dir->entry );
    dir->entry = NULL;
  }
  // update position offset
  dir->file.fpos = it->entry ? it->reference->file.fpos : dir->file.fsize;
  // copy over entry
  if ( it->entry ) {
    memcpy( dir->entry, it->entry, sizeof( fat_structure_directory_entry_t ) );
    dir->entry_pos = it->reference->file.fpos;
  }
  // copy over data
  if ( it->data ) {
    memcpy( dir->data, it->data, sizeof( fat_directory_data_t ) );
  }
  // finish iterator
  result = fat_iterator_directory_fini( it );
  free( it );
  // return success
  return result;
}

/**
 * @brief Rewind directory offset to beginning
 *
 * @param dir
 * @return int
 */
BFSFAT_EXPORT int fat_directory_rewind( fat_directory_t* dir ) {
  // validate parameter
  if ( ! dir ) {
    return EINVAL;
  }
  // reset offset information
  dir->file.fpos = 0;
  // free data and entry
  if ( dir->data ) {
    free( dir->data );
    dir->data = NULL;
  }
  if ( dir->entry ) {
    free( dir->entry );
    dir->entry = NULL;
  }
  // unload cached block
  int result = fat_block_unload( &dir->file );
  if ( EOK != result ) {
    return result;
  }
  // return success
  return EOK;
}

/**
 * @brief Function to get entry of directory by name
 *
 * @param dir
 * @param name
 * @return int
 */
BFSFAT_EXPORT int fat_directory_entry_by_name(
  fat_directory_t* dir,
  const char* name
) {
  // validate parameter
  if ( ! dir || ! name ) {
    return EINVAL;
  }
  // space for variable
  int result = fat_directory_rewind( dir );
  if ( EOK != result ) {
    return result;
  }
  // loop through entries
  while ( EOK == ( result = fat_directory_next_entry( dir ) ) ) {
    // handle nothing in there
    if ( ! dir->data && ! dir->entry ) {
      return ENOENT;
    }
    // having entry, check for match
    if ( 0 == strcasecmp( dir->data->name, name ) ) {
      return EOK;
    }
  }
  // return last error of next entry
  return EOK != result ? result : ENOENT;
}

/**
 * @brief Check whether entry is free or not
 *
 * @param entry
 * @param is_free
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_entry_is_free(
  fat_structure_directory_entry_t* entry,
  bool* is_free
) {
  // validate parameter
  if ( ! entry || ! is_free ) {
    return EINVAL;
  }
  // set free information
  *is_free = '\0' == entry->name[ 0 ]
    || FAT_DIRECTORY_ENTRY_ERASED_AVAILABLE == ( uint8_t )entry->name[ 0 ];
  // return success
  return EOK;
}

/**
 * @brief Check whether entry is a valid one
 *
 * @param entry
 * @param is_valid
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_entry_is_valid(
  fat_structure_directory_entry_t* entry,
  bool* is_valid
) {
  // validate parameter
  if ( ! entry || ! is_valid ) {
    return EINVAL;
  }
  bool is_free;
  // invalid if free
  int result = fat_directory_entry_is_free( entry, &is_free );
  if ( EOK != result ) {
    return result;
  }
  // found a valid entry
  *is_valid = (
      ! is_free
      && ! ( entry->attributes & FAT_DIRECTORY_FILE_ATTRIBUTE_VOLUME_ID )
    ) || (
      FAT_DIRECTORY_FILE_ATTRIBUTE_LONG_FILE_NAME == entry->attributes
      && ! is_free
    );
  // return success
  return EOK;
}

/**
 * @brief Check whether entry is a dot entry
 *
 * @param entry
 * @param is_dot
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_entry_is_dot(
  fat_structure_directory_entry_t* entry,
  bool* is_dot
) {
  // validate parameter
  if ( ! entry || ! is_dot ) {
    return EINVAL;
  }
  // set free information
  *is_dot = FAT_DIRECTORY_ENTRY_DOT_ENTRY == entry->name[ 0 ];
  // return success
  return EOK;
}

/**
 * @brief Extract directory short name
 *
 * @param entry
 * @param name
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_extract_name_short(
  fat_structure_directory_entry_t* entry,
  char* name
) {
  // validate parameter
  if ( ! entry || ! name ) {
    return EINVAL;
  }
  // allocate temporary space
  char* temporary = malloc( sizeof( char ) * 10 );
  if ( ! temporary ) {
    return ENOMEM;
  }
  // clear out space
  memset( temporary, 0, 10 * sizeof( char ) );
  // copy entry name to temporary
  strncpy( temporary, entry->name, 8 );
  // copy trimmed name to name
  strcpy( name, util_trim( temporary ) );
  // clear out space
  memset( temporary, 0, 10 * sizeof( char ) );
  // copy extension to temporary
  strncpy( temporary, entry->extension, 3 );
  // save result of trim in extra pointer
  char* trimmed = util_trim( temporary );
  // check for extension is not empty
  if ( strlen( trimmed ) ) {
    // add a dot as separator
    strcat( name, "." );
    // cat trimmed extension
    strcat( name, trimmed );
  }
  // free up temporary again
  free( temporary );
  // return success
  return EOK;
}

/**
 * @brief Method to extend a fat direcory by entries
 *
 * @param dir
 * @param buffer
 * @param size
 * @return int
 *
 * @internal Not usable with opened root directory interface
 */
BFSFAT_NO_EXPORT int fat_directory_extend(
  fat_directory_t* dir,
  void* buffer,
  uint64_t size
) {
  // validate parameter
  if ( ! dir || ! dir->file.mp->fs ) {
    return EINVAL;
  }
  // check for readonly
  fat_fs_t* fs = dir->file.mp->fs;
  if ( fs->read_only ) {
    return EROFS;
  }
  // handle root directory
  if ( dir && 0 == dir->file.cluster ) {
    return fat_rootdir_extend( dir, buffer, size );
  }
  // validate parameter
  if ( ! dir || ! buffer || ! dir->file.cluster || ! dir->file.fsize ) {
    return EINVAL;
  }
  int result;
  // load chain if necessary
  result = fat_cluster_load( fs, &dir->file );
  if ( EOK != result ) {
    return result;
  }
  uint64_t necessary_entry_count = size
    / sizeof( fat_structure_directory_entry_t );
  // allocate buffer
  fat_structure_directory_entry_t* entry = malloc( ( size_t )dir->file.fsize );
  if ( ! entry ) {
    return ENOMEM;
  }
  // clear out space
  memset( entry, 0, ( size_t )dir->file.fsize );
  // load whole directory
  dir->file.fpos = 0;
  uint64_t cluster_size = fs->superblock.sectors_per_cluster
    * fs->superblock.bytes_per_sector;
  /// FIXME: EXTEND DIRECTORY IF NECESSARY
  while ( dir->file.fpos < dir->file.fsize ) {
    // load fat block
    result = fat_block_load( &dir->file, cluster_size );
    if ( EOK != result ) {
      free( entry );
      return result;
    }
    if ( ! dir->file.block.data ) {
      free( entry );
      return result;
    }
    // copy over to buffer
    memcpy(
      ( uint8_t* )entry + dir->file.fpos,
      dir->file.block.data,
      ( size_t )cluster_size
    );
    dir->file.fpos += cluster_size;
  }
  // loop through root directory and try to find a entry
  fat_structure_directory_entry_t* current = entry;
  fat_structure_directory_entry_t* end = ( fat_structure_directory_entry_t* )(
    ( uint8_t* )entry + dir->file.fsize
  );
  fat_structure_directory_entry_t* start = NULL;
  uint64_t found_size = 0;
  while ( current < end ) {
    // handle found enough space
    if ( start && found_size == necessary_entry_count ) {
      break;
    }
    // get free
    bool is_free;
    result = fat_directory_entry_is_free( current, &is_free );
    // handle non free
    if ( ! is_free ) {
      found_size = 0;
      start = NULL;
    // handle free block
    } else if ( is_free ) {
      if ( ! start ) {
        start = current;
      }
      found_size++;
    }
    // increment current
    current++;
  }
  // handle not enough free stuff
  if ( ! start || found_size != necessary_entry_count ) {
    while ( found_size < necessary_entry_count ) {
      // save start offset
      uint64_t start_offset = ( uint64_t )( start - entry );
      if ( ! start ) {
        start_offset = ( uint64_t )( end - entry );
      }
      // save old size
      uint64_t old_size = dir->file.fsize;
      // extend
      result = fat_file_extend_cluster( &dir->file, 1 );
      if ( EOK != result ) {
        free( entry );
        return result;
      }
      dir->file.fsize += cluster_size;
      // realloc entry space
      void* tmp_entry_extended = realloc( entry, ( size_t )dir->file.fsize );
      // handle error
      if ( ! tmp_entry_extended ) {
        free( entry );
        return result;
      }
      // clear out additional space
      memset( ( uint8_t* )tmp_entry_extended + old_size, 0, ( size_t )cluster_size );
      // increase found count by newly added entries
      found_size += ( dir->file.fsize - old_size )
        / sizeof( fat_structure_directory_entry_t );
      // overwrite entry pointer
      entry = tmp_entry_extended;
      // reset start
      start = entry + start_offset;
    }
  }
  // handle not enough space
  if ( ! start ) {
    free( entry );
    return ENOSPC;
  }
  // copy over changes
  memcpy( start, buffer, ( size_t )size );
  // write block by block
  uint64_t block_count = dir->file.fsize / cluster_size;
  uint64_t block_current = 0;
  for ( uint64_t block_index = 0; block_index < block_count; block_index++ ) {
    // get block
    block_current = dir->file.chain[ block_index ];
    // translate to lba
    uint64_t lba;
    result = fat_cluster_to_lba( fs, block_current, &lba );
    if ( EOK != result ) {
      free( entry );
      return result;
    }
    // write cluster
    result = common_blockdev_bytes_write(
      fs->bdev,
      lba * fs->bdev->bdif->block_size,
      ( uint8_t* )entry + block_index * cluster_size,
      cluster_size
    );
    if ( EOK != result ) {
      free( entry );
      return result;
    }
  }
  // freeup entry again
  free( entry );
  // return success
  return EOK;
}

/**
 * @brief Calculate checksum out of name
 *
 * @param name
 * @param checksum
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_dentry_checksum(
  uint8_t* name,
  uint8_t* checksum
) {
  // validate parameter
  if ( ! name || ! checksum ) {
    return EINVAL;
  }
  *checksum = 0;
  // loop through short name
  for ( uint64_t idx = 11; idx != 0; idx-- ) {
    *checksum = ( uint8_t )( ( ( *checksum & 1 ) ? 0x80 : 0 )
      + ( *checksum >> 1 ) + *name++ );
  }
  // return success
  return EOK;
}

/**
 * @brief Extend update directory by adding path
 *
 * @param dir
 * @param name
 * @param cluster
 * @param directory
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_dentry_insert(
  fat_directory_t* dir,
  const char* name,
  uint64_t cluster,
  bool directory
) {
  if ( ! dir || ! name ) {
    return EINVAL;
  }
  fat_fs_t* fs = dir->file.mp->fs;
  // handle create flag with read only
  if ( fs->read_only ) {
    return EROFS;
  }
  int result;
  // save length
  size_t name_length = strlen( name );
  // handle invalid name length
  if ( ! name_length ) {
    return EINVAL;
  }
  uint64_t entry_size;
  void* entry_data = NULL;
  size_t extension_length = 0;
  const char* ext = strchr( name, '.' );
  const char* dot = ext;
  if ( ! directory && ext ) {
    ext++;
    extension_length = strlen( name ) - ( size_t )( ext - name );
    name_length -= ( extension_length + 1 );
  }
  // build directory entry
  if (
    8 >= name_length
    && ( directory || ( ! directory && 3 >= extension_length ) )
  ) {
    // short entry is enough
    fat_structure_directory_entry_t* data = ( fat_structure_directory_entry_t* )
      malloc( sizeof( *data ) );
    if ( ! data ) {
      return ENOMEM;
    }
    entry_size = sizeof( *data );
    memset( data, 0, ( size_t )entry_size );
    // copy over name
    const char* p = name;
    while ( *p && ( p - name ) < 8 ) {
      // additional anchor for non directories with an extension
      if ( ! directory && dot && p >= dot ) {
        break;
      }
      data->name[ ( p - name ) ] = *p;
      p++;
    }
    if ( ! directory && ext ) {
      p = ext;
      size_t ext_count = 0;
      while ( *p && ext_count < 3 ) {
        data->extension[ ext_count ] = *p;
        p++;
        ext_count++;
      }
    }
    // set directory flag
    if ( directory ) {
      data->attributes = FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY;
      data->file_size = 0;
    }
    // set first cluster
    data->first_cluster_lower = ( uint16_t )cluster;
    if ( FAT_FAT32 == fs->type ) {
      data->first_cluster_upper = ( uint16_t )( ( uint32_t )cluster >> 16 );
    }
    // set entry data
    entry_data = data;
  } else {
    if ( ! directory && ext ) {
      name_length += ( extension_length + 1 );
    }
    // calculate amount of long entries necessary
    size_t long_name_length = name_length / 13;
    if ( name_length % 13 ) {
      long_name_length++;
    }
    // add final short entry
    long_name_length++;
    // allocate
    fat_structure_directory_entry_long_t* data = ( fat_structure_directory_entry_long_t* )
      malloc( sizeof( *data ) * long_name_length );
    if ( ! data ) {
      return ENOMEM;
    }
    entry_size = sizeof( *data ) * long_name_length;
    memset( data, 0, ( size_t )entry_size );
    // build short name
    fat_structure_directory_entry_t* short_entry = ( fat_structure_directory_entry_t* )
      ( &data[ long_name_length - 1 ] );
    // set directory flag
    if ( directory ) {
      short_entry->attributes = FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY;
      short_entry->file_size = 0;
    }
    // set first cluster
    short_entry->first_cluster_lower = ( uint16_t )cluster;
    if ( FAT_FAT32 == fs->type ) {
      short_entry->first_cluster_upper = ( uint16_t )( ( uint32_t )cluster >> 16 );
    }
    strncpy( short_entry->name, name, 7 );
    short_entry->name[ 7 ] = '~';
    if ( ! directory && ext ) {
      if ( 3 >= extension_length ) {
        for ( size_t idx = 0; idx < extension_length; idx++ ) {
          short_entry->extension[ idx ] = ext[ idx ];
        }
      } else {
        short_entry->extension[ 0 ] = ext[ 0 ];
        short_entry->extension[ 1 ] = ext[ 1 ];
        short_entry->extension[ 2 ] = '~';
      }
    }
    // calculate checksum
    uint8_t checksum;
    result = fat_directory_dentry_checksum(
      ( uint8_t* )short_entry->name,
      &checksum
    );
    if ( EOK != result ) {
      free( data );
      return result;
    }
    // loop through long name parts and fill them
    for ( size_t index = 0; index < long_name_length - 1; index++ ) {
      uint8_t* base_offsetted = ( uint8_t* )( name + ( index * 13 ) );
      uint8_t* end = ( uint8_t* )( name + name_length );
      // order index
      size_t order_index = long_name_length - 2 - index;
      // set order information
      data[ order_index ].order = ( uint8_t )index + 1;
      // last entry is orred with 0x40
      if ( data[ order_index ].order == long_name_length - 1 ) {
        data[ order_index ].order |= 0x40;
      }
      data[ order_index ].attribute = FAT_DIRECTORY_FILE_ATTRIBUTE_LONG_FILE_NAME;
      data[ order_index ].checksum = checksum;
      // populate long entry names
      for (
        uint64_t local_index = 0;
        base_offsetted < end && local_index < 10;
        local_index += 2, base_offsetted++
      ) {
        data[ order_index ].first_five_two_byte[ local_index ] = *base_offsetted;
      }
      for (
        uint64_t local_index = 0;
        base_offsetted < end && local_index < 12;
        local_index += 2, base_offsetted++
      ) {
        data[ order_index ].next_six_two_byte[ local_index ] = *base_offsetted;
      }
      for (
        uint64_t local_index = 0;
        base_offsetted < end && local_index < 4;
        local_index += 2, base_offsetted++
      ) {
        data[ order_index ].final_two_byte[ local_index ] = *base_offsetted;
      }
    }
    // set entry data
    entry_data = data;
  }
  // insert directory
  result = fat_directory_extend( dir, entry_data, entry_size );
  if ( EOK != result ) {
    free( entry_data );
    return result;
  }
  // free up everything
  free( entry_data );
  return EOK;
}

/**
 * @brief Method to update specific directory structure entry
 *
 * @param dir
 * @param dentry
 * @param pos position in bytes where dentry starts
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_dentry_update(
  fat_directory_t* dir,
  fat_structure_directory_entry_t* dentry,
  uint64_t pos
) {
  // validate parameter
  if ( ! dir || ! dentry ) {
    return EINVAL;
  }
  // validate pos
  if ( pos >= dir->file.fsize || pos + sizeof( *dentry ) >= dir->file.fsize ) {
    return EINVAL;
  }
  // get fs and calculate cluster size
  common_mountpoint_t* mp = dir->file.mp;
  fat_fs_t* fs = mp->fs;
  uint64_t cluster_size = fs->superblock.sectors_per_cluster
    * fs->superblock.bytes_per_sector;
  // handle create flag with read only
  if ( fs->read_only ) {
    return EROFS;
  }
  // load block data
  int result = fat_block_load( &dir->file, cluster_size );
  if ( EOK != result ) {
    return result;
  }
  // calculate offset in block
  uint64_t offset = dir->file.fpos % cluster_size;
  // overwrite data
  memcpy( dir->file.block.data + offset, dentry, sizeof( *dentry ) );
  // write data again
  return fat_block_write( &dir->file, cluster_size );
}

/**
 * @brief Method to remove a directory entry
 *
 * @param dir
 * @param dentry
 * @param pos
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_dentry_remove(
  fat_directory_t* dir,
  fat_structure_directory_entry_t* dentry,
  uint64_t pos
) {
  // validate parameter
  if ( ! dir || ! dir->file.mp->fs || ! dir->file.fsize ) {
    return EINVAL;
  }
  // check for readonly
  fat_fs_t* fs = dir->file.mp->fs;
  if ( fs->read_only ) {
    return EROFS;
  }
  // handle root directory
  if ( dir && 0 == dir->file.cluster ) {
    return fat_rootdir_remove( dir, dentry, pos );
  }
  int result;
  // allocate buffer for file directory entries
  fat_structure_directory_entry_t* entry = malloc( ( size_t )dir->file.fsize );
  if ( ! entry ) {
    return ENOMEM;
  }
  // clear out space
  memset( entry, 0, ( size_t )dir->file.fsize );
  // reset file position
  dir->file.fpos = 0;
  // calculate cluster size
  uint64_t cluster_size = fs->superblock.sectors_per_cluster
    * fs->superblock.bytes_per_sector;
  // load whole directory
  while ( dir->file.fpos < dir->file.fsize ) {
    // load fat block
    result = fat_block_load( &dir->file, cluster_size );
    if ( EOK != result ) {
      free( entry );
      return result;
    }
    // handle no data
    if ( ! dir->file.block.data ) {
      free( entry );
      return result;
    }
    // copy over to buffer
    memcpy(
      ( uint8_t* )entry + dir->file.fpos,
      dir->file.block.data,
      ( size_t )cluster_size
    );
    // unload fat block
    result = fat_block_unload( &dir->file );
    if ( EOK != result ) {
      free( entry );
      return result;
    }
    // increase position
    dir->file.fpos += cluster_size;
  }
  // set start
  fat_structure_directory_entry_t* start = entry;
  // set current
  fat_structure_directory_entry_t* current = ( fat_structure_directory_entry_t* )(
    ( uint8_t* )entry + pos
  );
  uint64_t count = 0;
  // loop backwards until first possible non long name
  while( true ) {
    // decrement current
    current--;
    // increment count if long name
    if ( current->attributes == FAT_DIRECTORY_FILE_ATTRIBUTE_LONG_FILE_NAME ) {
      count++;
    }
    // handle beginning reached
    if (
      start >= current
      || current->attributes != FAT_DIRECTORY_FILE_ATTRIBUTE_LONG_FILE_NAME
    ) {
      break;
    }
  }
  // update position
  pos -= count * sizeof( fat_structure_directory_entry_t );
  // increment due to short entry
  count++;
  for ( uint64_t index = 0; index < count; index++ ) {
    // get entry to delete
    fat_structure_directory_entry_long_t* current2 =
      ( fat_structure_directory_entry_long_t* )start;
    current2 += ( pos / sizeof( fat_structure_directory_entry_long_t ) );
    current2 += index;
    // mark as deleted
    current2->order = FAT_DIRECTORY_ENTRY_ERASED_AVAILABLE;
  }
  // reset file position
  dir->file.fpos = 0;
  // load whole directory
  while ( dir->file.fpos < dir->file.fsize ) {
    // load fat block
    result = fat_block_load( &dir->file, cluster_size );
    if ( EOK != result ) {
      free( entry );
      return result;
    }
    // handle no data
    if ( ! dir->file.block.data ) {
      free( entry );
      return result;
    }
    // copy over to buffer
    memcpy(
      dir->file.block.data,
      ( uint8_t* )entry + dir->file.fpos,
      ( size_t )cluster_size
    );
    // write back fat block
    result = fat_block_write( &dir->file, cluster_size );
    if ( EOK != result ) {
      free( entry );
      return result;
    }
    // unload fat block
    result = fat_block_unload( &dir->file );
    if ( EOK != result ) {
      free( entry );
      return result;
    }
    // increase position
    dir->file.fpos += cluster_size;
  }
  // freeup entry again
  free( entry );
  // return success
  return EOK;
}

/**
 * @brief Get creation time of directory
 *
 * @param file
 * @param ctime
 * @return int
 */
BFSFAT_EXPORT int fat_directory_ctime( fat_directory_t* dir, time_t* ctime ) {
  return fat_directory_dentry_ctime( dir->entry, ctime );
}

/**
 * @brief Get modification time of directory
 *
 * @param file
 * @param ctime
 * @return int
 */
BFSFAT_EXPORT int fat_directory_mtime( fat_directory_t* dir, time_t* mtime ) {
  return fat_directory_dentry_mtime( dir->entry, mtime );
}

/**
 * @brief Get access time of directory
 *
 * @param file
 * @param ctime
 * @return int
 */
BFSFAT_EXPORT int fat_directory_atime( fat_directory_t* dir, time_t* atime ) {
  return fat_directory_dentry_atime( dir->entry, atime );
}

/**
 * @brief Get creation time of dentry
 *
 * @param file
 * @param ctime
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_dentry_ctime( fat_structure_directory_entry_t* dir, time_t* ctime ) {
  // validate parameter
  if ( ! dir || ! ctime ) {
    return EINVAL;
  }
  struct tm t = {
    .tm_sec = FAT_DIRECTORY_TIME_EXTRACT_SECOND( dir->creation_time ),
    .tm_min = FAT_DIRECTORY_TIME_EXTRACT_MINUTE( dir->creation_time ),
    .tm_hour = FAT_DIRECTORY_TIME_EXTRACT_HOUR( dir->creation_time ),
    .tm_mday = FAT_DIRECTORY_DATE_EXTRACT_DAY( dir->creation_date ),
    .tm_mon = FAT_DIRECTORY_DATE_EXTRACT_MONTH( dir->creation_date ),
    .tm_year = FAT_DIRECTORY_DATE_EXTRACT_YEAR( dir->creation_date ) + 80,
  };
  *ctime = mktime(&t);
  return EOK;
}

/**
 * @brief Get modification time of dentry
 *
 * @param file
 * @param ctime
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_dentry_mtime( fat_structure_directory_entry_t* dir, time_t* mtime ) {
  // validate parameter
  if ( ! dir || ! mtime ) {
    return EINVAL;
  }
  struct tm t = {
    .tm_sec = FAT_DIRECTORY_TIME_EXTRACT_SECOND( dir->last_modification_time ),
    .tm_min = FAT_DIRECTORY_TIME_EXTRACT_MINUTE( dir->last_modification_time ),
    .tm_hour = FAT_DIRECTORY_TIME_EXTRACT_HOUR( dir->last_modification_time ),
    .tm_mday = FAT_DIRECTORY_DATE_EXTRACT_DAY( dir->last_modification_date ),
    .tm_mon = FAT_DIRECTORY_DATE_EXTRACT_MONTH( dir->last_modification_date ),
    .tm_year = FAT_DIRECTORY_DATE_EXTRACT_YEAR( dir->last_modification_date ) + 80,
  };
  *mtime = mktime(&t);
  return EOK;
}

/**
 * @brief Get access time of dentry
 *
 * @param file
 * @param ctime
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_dentry_atime( fat_structure_directory_entry_t* dir, time_t* atime ) {
  // validate parameter
  if ( ! dir || ! atime ) {
    return EINVAL;
  }
  struct tm t = {
    .tm_sec = 0,
    .tm_min = 0,
    .tm_hour = 0,
    .tm_mday = FAT_DIRECTORY_DATE_EXTRACT_DAY( dir->last_accessed_date ),
    .tm_mon = FAT_DIRECTORY_DATE_EXTRACT_MONTH( dir->last_accessed_date ),
    .tm_year = FAT_DIRECTORY_DATE_EXTRACT_YEAR( dir->last_accessed_date ) + 80,
  };
  *atime = mktime(&t);
  return EOK;
}
