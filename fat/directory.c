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
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <common/mountpoint.h>
#include <common/errno.h>
#include <common/util.h>
#include <fat/directory.h>
#include <fat/type.h>
#include <fat/structure.h>
#include <fat/rootdir.h>
#include <fat/iterator.h>
#include <fat/cluster.h>
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
BFSFAT_NO_EXPORT int fat_directory_size( fat_directory_t* dir, uint64_t* size ) {
  // validate
  if ( ! dir || ! dir->file.mp->fs || ! size ) {
    return EINVAL;
  }
  // cache cluster
  uint64_t cluster = dir->file.cluster;
  // cache fs structure
  fat_fs_t* fs = dir->file.mp->fs;
  uint64_t sector_count = 0;
  while ( true ) {
    // increase sector count
    sector_count++;
    // get next cluster
    uint64_t next;
    int result = fat_cluster_next( fs, cluster, &next );
    if ( EOK != result ) {
      return result;
    }
    // handle chain end
    if (
      ( FAT_FAT12 == fs->type && next >= FAT_FAT12_CLUSTER_CHAIN_END )
      || ( FAT_FAT16 == fs->type && next >= FAT_FAT16_CLUSTER_CHAIN_END )
      || ( FAT_FAT32 == fs->type && next >= FAT_FAT32_CLUSTER_CHAIN_END )
    ) {
      break;
    }
    // overwrite cluster
    cluster = next;
  }
  // return size
  *size = sector_count * fs->superblock.bytes_per_sector;
  return EOK;
}

/**
 * @brief Remove a directory
 *
 * @param path
 * @param recursive
 * @return int
 */
BFSFAT_EXPORT int fat_directory_remove( const char* path, bool recursive ) {
  ( void )path;
  ( void )recursive;
  return ENOSYS;
}

/**
 * @brief Move a directory
 *
 * @param old_path
 * @param new_path
 * @return int
 */
BFSFAT_EXPORT int fat_directory_move( const char* old_path, const char* new_path ) {
  ( void )old_path;
  ( void )new_path;
  return ENOSYS;
}

/**
 * @brief Create a directory
 *
 * @param path
 * @return int
 */
BFSFAT_EXPORT int fat_directory_make( const char* path ) {
  ( void )path;
  return ENOSYS;
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

  // allocate iterator
  fat_iterator_directory_t* it = malloc( sizeof( *it ) );
  if ( ! it ) {
    free( duppath );
    return ENOMEM;
  }
  // clear space
  memset( it, 0, sizeof( *it ) );

  // parse sub directories
  //char* p = strtok( ( char* )( path + 1 ), CONFIG_PATH_SEPARATOR_STRING );
  p = strtok( p, CONFIG_PATH_SEPARATOR_STRING );
  while ( p != NULL ) {
    // setup iterator
    result = fat_iterator_directory_init( it, dir, 0 );
    if ( EOK != result ) {
      fat_iterator_directory_fini( it );
      free( it );
      free( duppath );
      return result;
    }
    // loop while an entry is existing
    while ( it->entry ) {
      // second case: matching entry
      if ( 0 == strcasecmp( it->data->name, p ) ) {
        break;
      }
      // get next iterator
      result = fat_iterator_directory_next( it );
      if ( EOK != result ) {
        fat_iterator_directory_fini( it );
        free( it );
        free( duppath );
        return result;
      }
    }
    // check for no entry
    if ( ! it->entry || !( it->entry->attributes & FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY ) ) {
      fat_iterator_directory_fini( it );
      free( it );
      free( duppath );
      return ENOENT;
    }
    uint32_t found_start_cluster = ( ( uint32_t )it->entry->first_cluster_upper << 16 )
      | ( uint32_t )it->entry->first_cluster_lower;
    // we found a matching entry, so close current directory
    fat_directory_close( dir );
    // prepare directory file object
    dir->file.mp = mp;
    dir->file.fpos = 0;
    dir->file.cluster = found_start_cluster;
    // extract offset and size
    result = fat_directory_size( dir, &size );
    if ( EOK != result ) {
      fat_iterator_directory_fini( it );
      free( it );
      free( duppath );
      return result;
    }
    // finish iterator
    result = fat_iterator_directory_fini( it );
    if ( EOK != result ) {
      fat_iterator_directory_fini( it );
      free( it );
      free( duppath );
      return result;
    }
    // calculate total size
    dir->file.fsize = size;
    // get next one
    p = strtok( NULL, CONFIG_PATH_SEPARATOR_STRING );
  }
  // free iterator and path duplicate
  free( it );
  free( duppath );
  // set offset to 0
  dir->file.fpos = 0;
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
  int result = fat_file_close( &dir->file );
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
  // allocate entry
  if ( ! dir->entry && it->entry ) {
    dir->entry = malloc( sizeof( fat_structure_directory_entry_t ) );
    if ( ! dir->entry ) {
      free( it );
      free( dir->data );
      return ENOMEM;
    }
    memset( dir->entry, 0, sizeof( fat_structure_directory_entry_t ) );
  }
  // update position offset
  dir->file.fpos = it->entry ? it->reference->file.fpos : dir->file.fsize;
  // copy over entry
  if ( it->entry ) {
    memcpy( dir->entry, it->entry, sizeof( fat_structure_directory_entry_t ) );
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
  int result;
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
  return result;
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
  bool is_dot;
  // invalid if free
  int result = fat_directory_entry_is_free( entry, &is_free );
  if ( EOK != result ) {
    return result;
  }
  // invalid if dot
  result = fat_directory_entry_is_dot( entry, &is_dot );
  if ( EOK != result ) {
    return result;
  }
  // found a valid entry
  *is_valid = (
      ! is_free
      && ! is_dot
      && ! ( entry->attributes & FAT_DIRECTORY_FILE_ATTRIBUTE_VOLUME_ID )
    ) || FAT_DIRECTORY_FILE_ATTRIBUTE_LONG_FILE_NAME == entry->attributes;
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
  strncpy( name, util_trim( temporary ), 8 );
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
    strncat( name, trimmed, 3 );
  }
  // free up temporary again
  free( temporary );
  // return success
  return EOK;
}
