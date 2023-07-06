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
#include <common/errno.h> // IWYU pragma: keep
#include <ext/superblock.h>
#include <ext/inode.h>
#include <ext/directory.h>
#include <ext/link.h>
#include <ext/type.h>
#include <ext/fs.h>
#include <ext/structure.h>
#include <ext/blockgroup.h>
#include <ext/bfsext_export.h>

/**
 * @brief Link directory
 *
 * @param dir
 * @param inode
 * @param number
 * @param path
 * @return int
 *
 * @todo evaluate type by mode
 */
BFSEXT_NO_EXPORT int ext_link_link(
  ext_fs_t* fs,
  ext_directory_t* dir,
  ext_structure_inode_t* inode,
  uint64_t number,
  const char* path
) {
  // validate
  if ( ! fs || ! dir || ! inode || ! path || ! number ) {
    return EINVAL;
  }
  // check if already existing
  int result = ext_directory_entry_by_name( dir, path );
  if ( ENOENT != result ) {
    return EOK != result ? result : EEXIST;
  }
  uint8_t file_type = 0;
  if (fs->superblock.s_rev_level && (
    fs->superblock.s_feature_incompat & EXT_SUPERBLOCK_EXT2_FEATURE_INCOMPAT_FILETYPE
  ) ) {
    switch ( inode->i_mode & 0xF000 ) {
      case EXT_INODE_EXT2_S_IFDIR:
        file_type = EXT_DIRECTORY_EXT2_FT_DIR;
        break;
      case EXT_INODE_EXT2_S_IFLNK:
        file_type = EXT_DIRECTORY_EXT2_FT_SYMLINK;
        break;
      case EXT_INODE_EXT2_S_IFREG:
        file_type = EXT_DIRECTORY_EXT2_FT_REG_FILE;
        break;
      default:
        file_type = EXT_DIRECTORY_EXT2_FT_UNKNOWN;
        break;
    }
  }
  uint64_t pos = 0;
  uint64_t new_entry_length;
  // get new entry size
  result = ext_directory_entry_size( strlen( path ) + 1, &new_entry_length );
  if ( EOK != result ) {
    return result;
  }
  // handle existing
  if ( dir->inode.i_size ) {
    // handle no data
    if ( ! dir->data ) {
      return EINVAL;
    }
    // loop through data
    while ( dir->inode.i_size && pos < dir->inode.i_size ) {
      // get entry
      ext_structure_directory_entry_t* entry = ( ext_structure_directory_entry_t* )
        &dir->data[ pos ];
      // calculate real entry size
      uint64_t entry_length;
      result = ext_directory_entry_size( entry->name_len, &entry_length );
      if ( EOK != result ) {
        return result;
      }
      // handle broken
      if ( 0 == entry->rec_len ) {
        return EINVAL;
      }
      // check if there is enough space
      if ( entry->rec_len - entry_length >= new_entry_length ) {
        ext_structure_directory_entry_t* new_entry = ( ext_structure_directory_entry_t* )
          &dir->data[ pos + entry_length ];
        // create new entry
        new_entry->name_len = ( uint8_t )strlen( path );
        memcpy( new_entry->name, path, new_entry->name_len + 1 );
        new_entry->inode = ( uint32_t )number;
        new_entry->rec_len = entry->rec_len - ( uint16_t )entry_length;
        new_entry->file_type = file_type;
        // shorten old entry
        entry->rec_len = ( uint16_t )entry_length;
        // write back data
        result = ext_inode_write_data( fs, &dir->inode, 0, dir->inode.i_size, dir->data );
        if ( EOK != result ) {
          return result;
        }
        // increment link count
        inode->i_links_count++;
        // write inode
        result = ext_inode_write_inode( fs, number, inode );
        if ( EOK != result ) {
          return result;
        }
        // return success
        return EOK;
      }
      // next position
      pos += entry->rec_len;
    }
  }
  // get block size
  uint64_t block_size;
  result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return EOK;
  }
  // allocate space for new block
  uint8_t* buffer = malloc( ( size_t )block_size );
  if ( ! buffer ) {
    return ENOMEM;
  }
  memset( buffer, 0, ( size_t )block_size );
  ext_structure_directory_entry_t* new_entry = ( ext_structure_directory_entry_t* )buffer;
  // create new entry
  new_entry->name_len = ( uint8_t )strlen( path );
  memcpy( new_entry->name, path, new_entry->name_len + 1 );
  new_entry->inode = ( uint32_t )number;
  new_entry->rec_len = ( uint16_t )block_size;
  new_entry->file_type = file_type;
  // write back data
  result = ext_inode_write_data( fs, &dir->inode, 0, block_size, buffer );
  if ( EOK != result ) {
    free( buffer );
    return result;
  }
  // increment link count
  inode->i_links_count++;
  // load directory if not loaded
  if ( ! dir->data ) {
    result = ext_directory_load( fs, dir, &dir->inode );
    if ( EOK != result ) {
      free( buffer );
      return result;
    }
  }
  // write inode
  result = ext_inode_write_inode( fs, number, inode );
  if ( EOK != result ) {
    free( buffer );
    return result;
  }
  free( buffer );
  // return success
  return EOK;
}

/**
 * @brief Unlink
 *
 * @param dir
 * @param path
 * @return int
 */
BFSEXT_NO_EXPORT int ext_link_unlink(
  ext_fs_t* fs,
  ext_directory_t* dir,
  const char* path
) {
  // validate
  if ( ! dir || ! path ) {
    return EINVAL;
  }
  // check if existing
  int result = ext_directory_entry_by_name( dir, path );
  if ( ENOENT == result ) {
    return EOK;
  } else if ( EOK != result ) {
    return result;
  }
  uint64_t pos = 0;
  // handle no data
  if ( ! dir->data ) {
    return EINVAL;
  }
  ext_structure_directory_entry_t* prev = NULL;
  ext_structure_directory_entry_t* next = NULL;
  // loop through data
  while ( dir->inode.i_size && pos < dir->inode.i_size ) {
    // get entry
    ext_structure_directory_entry_t* entry = ( ext_structure_directory_entry_t* )
      &dir->data[ pos ];
    if ( pos + entry->rec_len < dir->inode.i_size ) {
      next = ( ext_structure_directory_entry_t* )
        &dir->data[ pos + entry->rec_len ];
    }
    if (
      strlen( path ) == entry->name_len
      && 0 == strncmp( path, entry->name, entry->name_len )
      && entry->inode
    ) {
      // try to read inode
      ext_structure_inode_t inode;
      result = ext_inode_read_inode( fs, entry->inode, &inode );
      if ( EOK != result ) {
        return result;
      }
      // decrement link count
      inode.i_links_count--;
      // handle complete deletion
      if ( 0 == inode.i_links_count ) {
        // special handling for directories
        if ( ( inode.i_mode & 0xF000 ) == EXT_INODE_EXT2_S_IFDIR ) {
          // translate to local number
          uint64_t local_inode_number;
          result = ext_inode_get_local_inode( fs, entry->inode, &local_inode_number );
          if ( EOK != result ) {
            return result;
          }
          uint64_t bgnumber = local_inode_number / fs->superblock.s_inodes_per_group;
          // read block group
          ext_structure_block_group_descriptor_t bgd;
          result = ext_blockgroup_read( fs, bgnumber, &bgd );
          if ( EOK != result ) {
            return result;
          }
          // increment directories
          bgd.bg_used_dirs_count--;
          result = ext_blockgroup_write( fs, bgnumber, &bgd );
          if ( EOK != result ) {
            return result;
          }
        }
        // deallocate inode
        result = ext_inode_deallocate( fs, &inode, entry->inode );
        if ( EOK != result ) {
          return result;
        }
      }
      // update inode
      result = ext_inode_write_inode( fs, entry->inode, &inode );
      if ( EOK != result ) {
        return result;
      }
      // set entry inode to 0
      entry->inode = 0;
      // merge with previous if existing
      if ( prev ) {
        prev->rec_len += entry->rec_len;
      // try to merge with next one
      } else if ( next ) {
        uint16_t entry_length = entry->rec_len;
        memcpy( entry, next, next->rec_len );
        entry->rec_len += entry_length;
      }
      break;
    }
    // next position
    pos += entry->rec_len;
    // save previous
    prev = entry;
  }
  // write back data
  result = ext_inode_write_data( fs, &dir->inode, 0, dir->inode.i_size, dir->data );
  if ( EOK != result ) {
    return result;
  }
  // return success
  return EOK;
}
