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
#include <ctype.h>
#include <common/mountpoint.h>
#include <common/errno.h>
#include <common/util.h>
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
  *size = sector_count * fs->superblock.sectors_per_cluster
    * fs->superblock.bytes_per_sector;
  return EOK;
}

/**
 * @brief Remove a directory
 *
 * @param path
 * @return int
 *
 * @todo implement
 * @todo add test for removal of short named directory
 * @todo add test for removal of long named directory
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
  return ENOSYS;
}

/**
 * @brief Move a directory
 *
 * @param old_path
 * @param new_path
 * @return int
 *
 * @todo implement
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
  // get fs
  fat_fs_t* fs = mp->fs;
  // handle create flag with read only
  if ( fs->read_only ) {
    return EROFS;
  }
  return ENOSYS;
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
  // duplicate path for base
  char* pathdup_base = strdup( path );
  if ( ! pathdup_base ) {
    return ENOMEM;
  }
  // duplicate path for dir
  char* pathdup_dir = strdup( path );
  if ( ! pathdup_dir ) {
    free( pathdup_base );
    return ENOMEM;
  }
  // extract dirname
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
  // local variable for directory operations
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // try to open basename
  int result = fat_directory_open( &dir, dirpath );
  if ( EOK != result ) {
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  // check whether folder doesn't exist
  result = fat_directory_entry_by_name( &dir, base );
  if ( ENOENT != result ) {
    fat_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return EOK != result ? result : EEXIST;
  }
  // insert directory entry
  result = fat_directory_dentry_insert( &dir, base, true );
  if ( ENOENT != result ) {
    fat_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  // return success
  fat_directory_close( &dir );
  free( pathdup_base );
  free( pathdup_dir );
  return EOK;
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
    fat_fs_t* fs = mp->fs;
    uint32_t found_start_cluster = ( uint32_t )it->entry->first_cluster_lower;
    if ( FAT_FAT32 == fs->type ) {
      found_start_cluster |= ( ( uint32_t )it->entry->first_cluster_upper << 16 );
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
  // free up data
  if ( dir->data && ! it->data ) {
    free( dir->data );
    dir->data = NULL;
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
  uint64_t necessary_entry_count = size
    / sizeof( fat_structure_directory_entry_t );
  // allocate buffer
  fat_structure_directory_entry_t* entry = malloc( dir->file.fsize );
  if ( ! entry ) {
    return ENOMEM;
  }
  // clear out space
  memset( entry, 0, dir->file.fsize );
  // load whole directory
  dir->file.fpos = 0;
  if ( dir->file.fpos >= dir->file.fsize ) {
    free( entry );
    return EINVAL;
  }
  while ( dir->file.fpos < dir->file.fsize ) {
    // load fat block
    result = fat_block_load(
      &dir->file,
      fs->superblock.sectors_per_cluster * fs->superblock.bytes_per_sector
    );
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
      fs->superblock.sectors_per_cluster * fs->superblock.bytes_per_sector
    );
    dir->file.fpos += (
      fs->superblock.sectors_per_cluster * fs->superblock.bytes_per_sector
    );
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
      // realloc entry space
      void* tmp_entry_extended = realloc( entry, dir->file.fsize );
      // handle error
      if ( ! tmp_entry_extended ) {
        free( entry );
        return result;
      }
      // increase found count by newly added entries
      found_size += ( dir->file.fsize - old_size )
        / sizeof( fat_structure_directory_entry_t );
      // overwrite entry pointer
      entry = tmp_entry_extended;
      start = ( fat_structure_directory_entry_t* )(
        ( uint8_t* )entry + start_offset
      );
    }
  }
  // handle not enough space
  if ( ! start ) {
    free( entry );
    return ENOSPC;
  }
  // copy over changes
  memcpy( start, buffer, size );
  // write block by block
  uint64_t cluster_size = fs->superblock.sectors_per_cluster
    * fs->superblock.bytes_per_sector;
  uint64_t block_count = dir->file.fsize / cluster_size;
  uint64_t block_current = 0;
  for ( uint64_t block_index = 0; block_index < block_count; block_index++ ) {
    // get cluster to write
    if ( 0 == block_current ) {
      block_current = dir->file.cluster;
    // get next cluster to write
    } else {
      uint64_t next;
      result = fat_cluster_next( fs, block_current, &next );
      if ( EOK != result ) {
        free( entry );
        return result;
      }
      // overwrite block current
      block_current = next;
    }
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
    // increment index
    block_index++;
  }
  // freeup entry again
  free( entry );
  // return success
  return EOK;
}

/**
 * @brief Extend update directory by adding path
 *
 * @param dir
 * @param name
 * @param directory
 * @return int
 */
BFSFAT_NO_EXPORT int fat_directory_dentry_insert(
  fat_directory_t* dir,
  const char* name,
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
  // get a free cluster
  uint64_t free_cluster;
  int result = fat_cluster_get_free( fs, &free_cluster );
  if ( EOK != result ) {
    return result;
  }
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
      return result;
    }
    entry_size = sizeof( *data );
    memset( data, 0, entry_size );
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
    }
    // set first cluster
    data->first_cluster_lower = ( uint16_t )free_cluster;
    if ( FAT_FAT32 == fs->type ) {
      data->first_cluster_upper = ( uint16_t )( ( uint32_t )free_cluster >> 16 );
    }
    // set entry data
    entry_data = data;
  } else {
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
      return result;
    }
    entry_size = sizeof( *data ) * long_name_length;
    memset( data, 0, entry_size );
    // loop through long name parts and fill them
    for ( size_t index = 0; index < long_name_length - 1; index++ ) {
      data[ index ].order = ( uint8_t )index;
      uint8_t* base_offsetted = ( uint8_t* )( name + ( index * 13 ) );
      uint8_t* end = ( uint8_t* )( name + name_length );
      // populate long entry names
      for (
        uint64_t local_index = 0;
        base_offsetted < end && local_index < 10;
        local_index += 2, base_offsetted++
      ) {
        data[ index ].first_five_two_byte[ local_index ] = *base_offsetted;
      }
      for (
        uint64_t local_index = 0;
        base_offsetted < end && local_index < 12;
        local_index += 2, base_offsetted++
      ) {
        data[ index ].next_six_two_byte[ local_index ] = *base_offsetted;
      }
      for (
        uint64_t local_index = 0;
        base_offsetted < end && local_index < 4;
        local_index += 2, base_offsetted++
      ) {
        data[ index ].final_two_byte[ local_index ] = *base_offsetted;
      }
    }
    // build finalizing short name
    fat_structure_directory_entry_t* short_entry = ( fat_structure_directory_entry_t* )
      ( &data[ long_name_length - 1 ] );
    // set directory flag
    if ( directory ) {
      short_entry->attributes = FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY;
    }
    // set first cluster
    short_entry->first_cluster_lower = ( uint16_t )free_cluster;
    if ( FAT_FAT32 == fs->type ) {
      short_entry->first_cluster_upper = ( uint16_t )( ( uint32_t )free_cluster >> 16 );
    }
    strncpy( short_entry->name, name, 7 );
    short_entry->name[ 7 ] = '~';
    for ( uint64_t idx = 0; idx < 7; idx++ ) {
      short_entry->name[ idx ] = ( int )short_entry->name[ idx ];
    }
    if ( ! directory && ext ) {
      if ( 3 >= extension_length ) {
        strncpy( short_entry->extension, ext, 3 );
      } else {
        strncpy( short_entry->extension, ext, 2 );
        short_entry->extension[ 2 ] = '~';
      }
    }
    // set entry data
    entry_data = data;
  }
  // try to mark cluster as used
  result = fat_cluster_set_cluster( fs, free_cluster, FAT_CLUSTER_EOF );
  if ( EOK != result ) {
    free( entry_data );
    return result;
  }
  // insert directory
  result = fat_directory_extend( dir, entry_data, entry_size );
  if ( EOK != result ) {
    fat_cluster_set_cluster( fs, free_cluster, FAT_CLUSTER_UNUSED );
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
 *
 * @todo test either directly or indirectly via file tests
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
  // load block data
  int result = fat_block_load( &dir->file, cluster_size );
  if ( EOK != result ) {
    return result;
  }
  // calculate offset in block
  uint64_t offset = dir->file.fpos % cluster_size;
  // overwrite data
  memcpy( dir->file.block.data + offset, dentry, cluster_size );
  // write data again
  return fat_block_write( &dir->file, cluster_size );
}
