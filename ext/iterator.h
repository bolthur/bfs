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

/** @file ext/iterator.h */

#include <stdint.h>
#include <ext/structure.h>
#include <ext/type.h>

#ifndef _EXT_ITERATOR_H
#define _EXT_ITERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined( _BFS_COMPILING )
  /** @brief Directory iterator information */
  typedef struct ext_directory_iterator {
    /** @brief Directory being iterated */
    ext_directory_t* reference;
    /** @brief Current directory name */
    #ifdef __cplusplus
      char name[255];
    #else
      char name[];
    #endif
  } ext_iterator_directory_t;

  int ext_iterator_directory_init( ext_iterator_directory_t* it, ext_directory_t* dir, uint64_t pos );
  int ext_iterator_directory_next( ext_iterator_directory_t* it );
  int ext_iterator_directory_seek( ext_iterator_directory_t* it, uint64_t pos );
  int ext_iterator_directory_set( ext_iterator_directory_t* it, uint64_t block_size );
  int ext_iterator_directory_fini( ext_iterator_directory_t* it );
#endif

#ifdef __cplusplus
}
#endif

#endif
