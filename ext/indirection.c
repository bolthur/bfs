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
#include <stdint.h>
#include <common/errno.h> // IWYU pragma: keep
#include <ext/superblock.h>
#include <ext/indirection.h>
#include <ext/fs.h>
#include <ext/bfsext_export.h>

/**
 * @brief Helper to get indirection level
 *
 * @param fs
 * @param block
 * @param direct_block
 * @param indirect_block
 * @param level
 * @return int
 */
BFSEXT_NO_EXPORT int ext_indirection_level(
  ext_fs_t* fs,
  uint64_t block,
  uint64_t* direct_block,
  uint64_t* indirect_block,
  uint64_t* level
) {
  // validate
  if ( ! fs || ! direct_block || ! indirect_block || ! level ) {
    return EINVAL;
  }
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // evaluate indirection level
  if ( 12 > block ) {
    *direct_block = block;
    *indirect_block = 0;
    *level = 0;
  // singly indirection
  } else if ( ( block_size / 4 + 12 ) > block ) {
    *direct_block = 12;
    *indirect_block = block - 12;
    *level = 1;
  // doubly indirection
  } else if ( ( block_size / 4 ) * ( block_size / 4 + 1 ) + 12 > block ) {
    *direct_block = 13;
    *indirect_block = block - 12 - ( block_size / 4 );
    *level = 2;
    // triply indirection
  } else {
    *direct_block = 14;
    *indirect_block = block - 12 - ( block_size / 4 ) * ( block_size / 4 + 1);
    *level = 3;
  }
  // return success
  return EOK;
}
