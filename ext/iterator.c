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

#include <stdlib.h>
#include <common/errno.h>
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
  it->name = NULL;
  // return success
  return EOK;
}

/**
 * @brief Next entry by iterator
 *
 * @param it
 * @return int
 */
BFSEXT_NO_EXPORT int ext_iterator_directory_next( ext_iterator_directory_t* it ) {
  ( void )it;
  return EINVAL;
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
  ( void )it;
  ( void )pos;
  return EINVAL;
}

/**
 * @brief Iterator set
 *
 * @param it
 * @param block_size
 * @return int
 */
BFSEXT_NO_EXPORT int ext_iterator_directory_set(
  ext_iterator_directory_t* it,
  uint64_t block_size
) {
  ( void )it;
  ( void )block_size;
  return EINVAL;
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
  // clear name
  if ( it->name ) {
    free( it->name );
    it->name = NULL;
  }
  // return success
  return EOK;
}
