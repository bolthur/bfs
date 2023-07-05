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

#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <common/errno.h>
#include <common/transaction.h>
#include <ext/inode.h>
#include <ext/iterator.h>
#include <ext/directory.h>
#include <ext/superblock.h>
#include <ext/bfsext_export.h>
#include <ext/blockgroup.h>
#include <ext/link.h>

/**
 * @brief Load ext directory data
 *
 * @param fs
 * @param dir
 * @param inode
 * @return int
 */
BFSEXT_NO_EXPORT int ext_directory_load(
  ext_fs_t* fs,
  ext_directory_t* dir,
  ext_structure_inode_t* inode
) {
  // validate parameter
  if ( ! fs || ! dir || ! inode ) {
    return EINVAL;
  }
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // allocate data
  dir->data = malloc( inode->i_size );
  if ( ! dir->data ) {
    return ENOMEM;
  }
  // read inode data
  result = ext_inode_read_data( fs, inode, 0, inode->i_size, dir->data );
  if ( EOK != result ) {
    free( dir->data );
    return result;
  }
  // set inode
  memcpy( &dir->inode, inode, sizeof( *inode ) );
  // return success
  return EOK;
}

/**
 * @brief Remove directory
 *
 * @param path
 * @return int
 */
BFSEXT_EXPORT int ext_directory_remove( const char* path ) {
  // validate parameter
  if ( ! path ) {
    return EINVAL;
  }
  common_mountpoint_t* mp = common_mountpoint_find( path );
  if ( ! mp ) {
    return ENOMEM;
  }
  ext_fs_t* fs = mp->fs;
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
  ext_directory_t* dir = malloc( sizeof( *dir ) );
  if ( ! dir ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    return ENOMEM;
  }
  // clear out
  memset( dir, 0, sizeof( *dir ) );
  // first open complete path
  result = ext_directory_open( dir, path );
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
    ext_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return ENOTEMPTY;
  }
  // unlink
  result = ext_link_unlink( fs, dir, "." );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    ext_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return ENOTEMPTY;
  }
  result = ext_link_unlink( fs, dir, ".." );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    ext_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return ENOTEMPTY;
  }
  // close directory again
  result = ext_directory_close( dir );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // open parent directory directory
  result = ext_directory_open( dir, dirpath );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // get entry by name
  result = ext_directory_entry_by_name( dir, base );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    ext_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // try to remove it
  result = ext_link_unlink( fs, dir, base );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    ext_directory_close( dir );
    free( pathdup_base );
    free( pathdup_dir );
    free( dir );
    return result;
  }
  // close directory
  result = ext_directory_close( dir );
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
 * @brief Move directory
 *
 * @param old_path
 * @param new_path
 * @return int
 *
 * @todo add transaction
 */
BFSEXT_EXPORT int ext_directory_move( const char* old_path, const char* new_path ) {
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
  ext_fs_t* fs_old = mp->fs;
  ext_fs_t* fs_new = mp->fs;
  // handle create flag with read only
  if ( fs_old->read_only || fs_new->read_only ) {
    return EROFS;
  }
  // variables for source and target dir
  ext_directory_t source, target;
  memset( &source, 0, sizeof( source ) );
  memset( &target, 0, sizeof( target ) );
  // open source directory
  int result = ext_directory_open( &source, old_path );
  if ( EOK != result ) {
    return result;
  }
  // check if empty ( 2 entries . and .. are allowed )
  if ( 2 != source.entry_size ) {
    // close directory
    ext_directory_close( &source );
    // return not empty
    return ENOTEMPTY;
  }
  // close directory again
  result = ext_directory_close( &source );
  if ( EOK != result ) {
    return result;
  }
  // try to open target directory
  result = ext_directory_open( &target, new_path );
  if ( ENOENT != result ) {
    if ( EOK == result ) {
      ext_directory_close( &target );
    }
    return EEXIST;
  }
  // close directory again
  result = ext_directory_close( &target );
  if ( EOK != result ) {
    return result;
  }

  // duplicate paths
  char* dup_old_base = strdup( old_path );
  if ( ! dup_old_base ) {
    return ENOMEM;
  }
  char* dup_old_dir = strdup( old_path );
  if ( ! dup_old_dir ) {
    free( dup_old_base );
    return ENOMEM;
  }
  char* dup_new_base = strdup( new_path );
  if ( ! dup_new_base ) {
    free( dup_old_base );
    free( dup_old_dir );
    return ENOMEM;
  }
  char* dup_new_dir = strdup( new_path );
  if ( ! dup_new_dir ) {
    free( dup_old_base );
    free( dup_old_dir );
    free( dup_new_base );
    return ENOMEM;
  }
  // get dirnames ( parent directory )
  char* base_old_path = basename( dup_old_base );
  char* dir_old_path = dirname( dup_old_dir );
  char* base_new_path = basename( dup_new_base );
  char* dir_new_path = dirname( dup_new_dir );
  // check for unsupported
  if ( '.' == *dir_old_path || '.' == *dir_new_path ) {
    free( dup_old_base );
    free( dup_old_dir );
    free( dup_new_base );
    free( dup_new_dir );
    return ENOTSUP;
  }
  // open source directory
  result = ext_directory_open( &source, old_path );
  if ( EOK != result ) {
    free( dup_old_base );
    free( dup_old_dir );
    free( dup_new_base );
    free( dup_new_dir );
    return result;
  }
  // open base directory
  ext_directory_t new_dir_dir;
  memset( &new_dir_dir, 0, sizeof( new_dir_dir ) );
  result = ext_directory_open( &new_dir_dir, dir_new_path );
  if ( EOK != result ) {
    free( dup_old_base );
    free( dup_old_dir );
    free( dup_new_base );
    free( dup_new_dir );
    return result;
  }
  // unlink .. and link it again
  result = ext_link_unlink( source.mp->fs, &source, ".." );
  if ( EOK != result ) {
    free( dup_old_base );
    free( dup_old_dir );
    free( dup_new_base );
    free( dup_new_dir );
    return result;
  }
  result = ext_link_link(
    source.mp->fs,
    &source,
    &new_dir_dir.inode,
    new_dir_dir.inode_number,
    ".."
  );
  if ( EOK != result ) {
    free( dup_old_base );
    free( dup_old_dir );
    free( dup_new_base );
    free( dup_new_dir );
    return result;
  }
  // link in dir dir
  result = ext_link_link(
    new_dir_dir.mp->fs,
    &new_dir_dir,
    &source.inode,
    source.inode_number,
    base_new_path
  );
  if ( EOK != result ) {
    free( dup_old_base );
    free( dup_old_dir );
    free( dup_new_base );
    free( dup_new_dir );
    return result;
  }
  // close directories
  result = ext_directory_close( &source );
  if ( EOK != result ) {
    free( dup_old_base );
    free( dup_old_dir );
    free( dup_new_base );
    free( dup_new_dir );
    return result;
  }
  result = ext_directory_close( &new_dir_dir );
  if ( EOK != result ) {
    free( dup_old_base );
    free( dup_old_dir );
    free( dup_new_base );
    free( dup_new_dir );
    return result;
  }
  // open old base directory
  ext_directory_t old_dir_dir;
  memset( &old_dir_dir, 0, sizeof( old_dir_dir ) );
  result = ext_directory_open( &old_dir_dir, dir_old_path );
  if ( EOK != result ) {
    free( dup_old_base );
    free( dup_old_dir );
    free( dup_new_base );
    free( dup_new_dir );
    return result;
  }
  // unlink
  result = ext_link_unlink( old_dir_dir.mp->fs, &old_dir_dir, base_old_path );
  if ( EOK != result ) {
    free( dup_old_base );
    free( dup_old_dir );
    free( dup_new_base );
    free( dup_new_dir );
    return result;
  }
  free( dup_old_base );
  free( dup_old_dir );
  free( dup_new_base );
  free( dup_new_dir );
  // close old dir again
  result = ext_directory_close( &old_dir_dir );
  if ( EOK != result ) {
    return result;
  }
  // return success
  return EOK;
}

/**
 * @brief Create directory
 *
 * @param path
 * @return int
 */
BFSEXT_EXPORT int ext_directory_make( const char* path ) {
  // validate parameter
  if ( ! path ) {
    return EINVAL;
  }
  common_mountpoint_t* mp = common_mountpoint_find( path );
  if ( ! mp ) {
    return ENOMEM;
  }
  ext_fs_t* fs = mp->fs;
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
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // try to open basename
  result = ext_directory_open( &dir, dirpath );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  // ensure that the folder doesn't exist
  result = ext_directory_entry_by_name( &dir, base );
  if ( ENOENT != result ) {
    common_transaction_rollback( fs->bdev );
    ext_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return EOK != result ? result : EEXIST;
  }
  // allocate new inode
  ext_structure_inode_t inode;
  uint64_t number;
  memset( &inode, 0, sizeof( inode ) );
  result = ext_inode_allocate( fs, &inode, &number );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    ext_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  // prepare inode
  inode.i_mode = EXT_INODE_EXT2_S_IFDIR;
  inode.i_dtime = 0;
  // temporary new directory
  ext_directory_t newdir = {
    .inode = inode,
    .inode_number = number,
    .data = NULL,
    .entry = NULL,
    .pos = 0,
    .entry_size = 0,
    .mp = mp,
  };
  // link in parent
  result = ext_link_link( fs, &dir, &inode, number, base );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    ext_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  // create . and ..
  result = ext_link_link( fs, &newdir, &inode, number, "." );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    ext_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  result = ext_link_link( fs, &newdir, &dir.inode, dir.inode_number, ".." );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    ext_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  // translate to local number
  uint64_t local_inode_number;
  result = ext_inode_get_local_inode( fs, number, &local_inode_number );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    ext_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  uint64_t bgnumber = local_inode_number / fs->superblock.s_inodes_per_group;
  // read block group
  ext_structure_block_group_descriptor_t bgd;
  result = ext_blockgroup_read(fs, bgnumber, &bgd );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    ext_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  // increment directories
  bgd.bg_used_dirs_count++;
  result = ext_blockgroup_write(fs, bgnumber, &bgd );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    ext_directory_close( &dir );
    free( pathdup_base );
    free( pathdup_dir );
    return result;
  }
  free( pathdup_base );
  free( pathdup_dir );
  // close directory
  result = ext_directory_close( &dir );
  if ( EOK != result ) {
    common_transaction_rollback( fs->bdev );
    return result;
  }
  result = ext_directory_close( &newdir );
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
BFSEXT_EXPORT int ext_directory_open( ext_directory_t* dir, const char* path ) {
  // validate
  if ( ! dir || ! path || *path != CONFIG_PATH_SEPARATOR_CHAR ) {
    return EINVAL;
  }
  // try to get mountpoint by path
  common_mountpoint_t* mp = common_mountpoint_find( path );
  if ( ! mp ) {
    return ENOENT;
  }

  uint64_t parent_no = EXT_INODE_EXT2_ROOT_INO;
  ext_structure_inode_t parent;
  ext_directory_t dirent;

  // create path duplicate
  const char* real_path = path + strlen( mp->name ) - 1;
  char* duppath = strdup( real_path );
  if ( ! duppath ) {
    return ENOMEM;
  }
  char* p = duppath;

  p = strtok( p, CONFIG_PATH_SEPARATOR_STRING );
  while ( p != NULL ) {
    // only valid when there is something
    if ( strlen( p ) ) {
      // read inode
      int result = ext_inode_read_inode( mp->fs, parent_no, &parent );
      if ( EOK != result ) {
        if ( dir->data ) {
          free( dir->data );
          dir->data = NULL;
        }
        free( duppath );
        return result;
      }
      // load directory
      memset( &dirent, 0, sizeof( dirent ) );
      result = ext_directory_load( mp->fs, &dirent, &parent );
      if ( EOK != result ) {
        if ( dir->data ) {
          free( dir->data );
          dir->data = NULL;
        }
        free( duppath );
        return result;
      }
      // get entry
      result = ext_directory_entry_by_name( &dirent, p );
      if ( EOK != result ) {
        if ( dir->data ) {
          free( dir->data );
          dir->data = NULL;
        }
        free( dirent.data );
        free( duppath );
        return result;
      }
      // overwrite parent number
      parent_no = dirent.entry->inode;
      // handle already allocated
      if ( dir->data ) {
        free( dir->data );
        dir->data = NULL;
      }
      // copy over data
      dir->data = malloc( dirent.inode.i_size );
      if ( ! dir->data ) {
        free( duppath );
        free( dirent.data );
        return ENOMEM;
      }
      memcpy( dir->data, dirent.data, dirent.inode.i_size );
      memcpy( &dir->inode, &dirent.inode, sizeof( dir->inode ) );
      dir->inode_number = dirent.entry->inode;
      // free up data again
      free( dirent.data );
    }
    // get next one
    p = strtok( NULL, CONFIG_PATH_SEPARATOR_STRING );
  }
  // free duplicate
  free( duppath );

  // read inode
  int result = ext_inode_read_inode( mp->fs, parent_no, &parent );
  if ( EOK != result ) {
    if ( dir->data ) {
      free( dir->data );
      dir->data = NULL;
    }
    return result;
  }
  // load directory
  memset( &dirent, 0, sizeof( dirent ) );
  result = ext_directory_load( mp->fs, &dirent, &parent );
  if ( EOK != result ) {
    if ( dir->data ) {
      free( dir->data );
      dir->data = NULL;
    }
    return result;
  }
  // handle already allocated
  if ( dir->data ) {
    free( dir->data );
    dir->data = NULL;
  }
  // copy over data
  dir->data = malloc( dirent.inode.i_size );
  if ( ! dir->data ) {
    free( dirent.data );
    return ENOMEM;
  }
  memcpy( dir->data, dirent.data, dirent.inode.i_size );
  memcpy( &dir->inode, &dirent.inode, sizeof( dir->inode ) );
  dir->inode_number = parent_no;
  // free up data again
  free( dirent.data );
  // copy over mountpoint
  dir->mp = mp;

  // rewind
  result = ext_directory_rewind(dir);
  if ( EOK != result ) {
    ext_directory_close( dir );
    return result;
  }
  // count entries
  uint64_t entry_size = 0;
  while ( true ) {
    // get next entry
    result = ext_directory_next_entry( dir );
    // handle error
    if ( EOK != result ) {
      ext_directory_close( dir );
      return result;
    // handle end reached
    } else if ( ! dir->entry ) {
      break;
    }
    // increment entry size
    entry_size++;
  }
  // rewind again
  result = ext_directory_rewind(dir);
  if ( EOK != result ) {
    ext_directory_close( dir );
    return result;
  }
  // set entry size
  dir->entry_size = entry_size;

  // return success
  return EOK;
}

/**
 * @brief Close opened directory
 *
 * @param dir
 * @return int
 */
BFSEXT_EXPORT int ext_directory_close( ext_directory_t* dir ) {
  if ( ! dir ) {
    return EINVAL;
  }
  if ( dir->data ) {
    free( dir->data );
    dir->data = NULL;
  }
  if ( dir->entry ) {
    dir->entry = NULL;
  }
  return EOK;
}

/**
 * @brief Get next entry
 *
 * @param dir
 * @return int
 */
BFSEXT_EXPORT int ext_directory_next_entry( ext_directory_t* dir ) {
  // handle end reached
  if ( dir->pos >= dir->inode.i_size ) {
    // reset entry
    if ( dir->entry ) {
      dir->entry = NULL;
    }
    // return success
    return EOK;
  }
  // allocate iterator
  ext_iterator_directory_t* it = malloc( sizeof( *it ) );
  if ( ! it ) {
    return ENOMEM;
  }
  // clear out
  memset( it, 0, sizeof( *it ) );
  // setup iterator
  int result = ext_iterator_directory_init( it, dir, dir->pos );
  if ( EOK != result ) {
    free( it );
    return result;
  }
  if ( dir->entry ) {
    // get next iterator
    result = ext_iterator_directory_next( it );
    if ( EOK != result ) {
      free( it );
      return result;
    }
  }
  // set entry and position
  dir->entry = NULL;
  dir->pos = dir->inode.i_size;
  if ( it->entry ) {
    dir->entry = it->entry;
    dir->pos = it->pos;
  }
  // finish iterator
  result = ext_iterator_directory_fini( it );
  free( it );
  // return success
  return result;
}

/**
 * @brief Rewind directory
 *
 * @param dir
 * @return int
 */
BFSEXT_EXPORT int ext_directory_rewind( ext_directory_t* dir ) {
  // validate parameter
  if ( ! dir ) {
    return EINVAL;
  }
  // reset offset information
  dir->pos = 0;
  if ( dir->entry ) {
    dir->entry = NULL;
  }
  // return success
  return EOK;
}

/**
 * @brief Get directory entry by name
 *
 * @param dir
 * @param name
 * @return int
 */
BFSEXT_EXPORT int ext_directory_entry_by_name( ext_directory_t* dir, const char* name ) {
  // validate parameter
  if ( ! dir || ! name ) {
    return EINVAL;
  }
  // space for variable
  int result = ext_directory_rewind( dir );
  if ( EOK != result ) {
    return result;
  }
  // loop through entries
  while ( EOK == ( result = ext_directory_next_entry( dir ) ) ) {
    // handle nothing in there
    if ( ! dir->entry ) {
      return ENOENT;
    }
    // having entry, check for match
    if (
      strlen( name ) == dir->entry->name_len
      && 0 == strncmp( dir->entry->name, name, dir->entry->name_len )
    ) {
      return EOK;
    }
  }
  // return last error of next entry
  return EOK != result ? result : ENOENT;
}

/**
 * @brief calculate directory entry size
 *
 * @param name_length
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_directory_entry_size(
  uint64_t name_length,
  uint64_t* value
) {
  // validate
  if ( ! value ) {
    return EINVAL;
  }
  // calculate length
  uint64_t length = name_length + sizeof( ext_structure_directory_entry_t );
  if ( length % 4 ) {
    length += 4 - length % 4;
  }
  // save
  *value = length;
  return EOK;
}
