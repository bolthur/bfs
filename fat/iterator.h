/**
 * Copyright (C) 2022 bolthur project.
 *
 * This file is part of bolthur/bfs.
 *
 * bolthur/bfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bolthur/bfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bolthur/bfs.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <fat/structure.h>
#include <fat/directory.h>
#include <fat/type.h>

#ifndef _FAT_ITERATOR_H
#define _FAT_ITERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Directory iterator information
 */
typedef struct fat_directory_iterator {
  /**
   * @brief Directory being iterated
   */
  fat_directory_t* reference;
  /**
   * @brief Some data block used during iteration
   */
  fat_block_t block;
  /**
   * @brief Current directory entry
   */
  fat_structure_directory_entry_t* entry;
  /**
   * @brief Current directory data
   */
  fat_directory_data_t* data;
  /**
   * @brief Iterator offset
   */
  uint64_t offset;
} fat_iterator_directory_t;

int fat_iterator_directory_init( fat_iterator_directory_t* it, fat_directory_t* dir, uint64_t pos );
int fat_iterator_directory_next( fat_iterator_directory_t* it );
int fat_iterator_directory_seek( fat_iterator_directory_t* it, uint64_t pos );
int fat_iterator_directory_set( fat_iterator_directory_t* it, uint64_t block_size );
int fat_iterator_directory_fini( fat_iterator_directory_t* it );

#ifdef __cplusplus
}
#endif

#endif
