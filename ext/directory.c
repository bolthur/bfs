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
#include <common/errno.h>
#include <ext/inode.h>
#include <ext/iterator.h>
#include <ext/directory.h>
#include <ext/superblock.h>
#include <ext/bfsext_export.h>

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
  return ENOTSUP;
}

/**
 * @brief Move directory
 *
 * @param old_path
 * @param new_path
 * @return int
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
  return ENOTSUP;
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
  return ENOTSUP;
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
  // free up data again
  free( dirent.data );
  // copy over mountpoint
  dir->mp = mp;

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
