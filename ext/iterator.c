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
#include <stdlib.h>
#include <common/errno.h> // IWYU pragma: keep
#include <ext/structure.h>
#include <ext/type.h>
#include <ext/iterator.h>
#include <ext/bfsext_export.h>

/**
 * @brief Setup iterator
 *
 * @param it
 * @param dir
 * @param pos
 * @return int
 */
BFSEXT_NO_EXPORT int ext_iterator_directory_init(
  ext_iterator_directory_t* it,
  ext_directory_t* dir,
  uint64_t pos
) {
  // validate parameter
  if ( ! it || ! dir ) {
    return EINVAL;
  }
  // setup iterator
  it->pos = pos;
  it->reference = dir;
  it->entry = dir->entry;
  // return result of seek
  return ext_iterator_directory_seek( it, pos );
}

/**
 * @brief Next entry by iterator
 *
 * @param it
 * @return int
 */
BFSEXT_NO_EXPORT int ext_iterator_directory_next( ext_iterator_directory_t* it ) {
  // skip value is sizeof entry structure
  uint64_t skip = it->entry->rec_len;
  // get next directory
  return ext_iterator_directory_seek( it, it->pos + skip );
}

/**
 * @brief Seek
 *
 * @param it
 * @param pos
 * @return int
 */
BFSEXT_NO_EXPORT int ext_iterator_directory_seek(
  ext_iterator_directory_t* it,
  uint64_t pos
) {
  // validate parameter
  if ( ! it ) {
    return EINVAL;
  }
  // handle position exceeds size
  if ( pos >= it->reference->inode.i_size ) {
    // set fpos to position
    it->pos = it->reference->inode.i_size;
    it->entry = NULL;
    // return no entry
    return EOK;
  }
  // set iterator data
  it->pos = pos;
  it->entry = NULL;
  // loop through data and try to find entry
  while ( pos < it->reference->inode.i_size ) {
    // get entry
    ext_structure_directory_entry_t* entry = ( ext_structure_directory_entry_t* )(
      &it->reference->data[ pos ]
    );
    // increment position
    pos += entry->rec_len;
    // handle unknown
    if ( EXT_DIRECTORY_EXT2_FT_UNKNOWN == entry->file_type ) {
      continue;
    }
    // handle invalid size
    if ( 0 == entry->rec_len ) {
      return EIO;
    }
    // set entry pointer and position
    it->entry = entry;
    it->pos = pos - entry->rec_len;
    break;
  }
  // handle nothing found
  if ( ! it->entry ) {
    // set fpos to position
    it->pos = it->reference->inode.i_size;
    it->entry = NULL;
  }
  // return success
  return EOK;
}

/**
 * @brief Iterator finish
 *
 * @param it
 * @return int
 */
BFSEXT_NO_EXPORT int ext_iterator_directory_fini( ext_iterator_directory_t* it ) {
  // validate parameter
  if ( ! it ) {
    return EINVAL;
  }
  it->entry = NULL;
  it->reference = NULL;
  // return success
  return EOK;
}
