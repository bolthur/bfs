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

#include <common/errno.h>
#include <ext/superblock.h>
#include <ext/block.h>
#include <ext/bfsext_export.h>

/**
 * @brief Reads block bitmap
 *
 * @param fs
 * @param descriptor
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_block_read_bitmap(
  ext_fs_t* fs,
  ext_structure_block_group_descriptor_t* descriptor,
  uint32_t* value
) {
  // validate
  if ( ! fs || ! descriptor || ! value ) {
    return EINVAL;
  }
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // load block group
  result = common_blockdev_bytes_read(
    fs->bdev,
    descriptor->bg_block_bitmap * block_size,
    value,
    block_size
  );
  if ( EOK != result ) {
    return result;
  }
  // return success
  return EOK;
}

/**
 * @brief Write back block bitmap
 *
 * @param fs
 * @param descriptor
 * @param value
 * @return int
 */
BFSEXT_NO_EXPORT int ext_block_write_bitmap(
  ext_fs_t* fs,
  ext_structure_block_group_descriptor_t* descriptor,
  uint32_t* value
) {
  // validate
  if ( ! fs || ! descriptor || ! value ) {
    return EINVAL;
  }
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // load block group
  result = common_blockdev_bytes_write(
    fs->bdev,
    descriptor->bg_block_bitmap * block_size,
    value,
    block_size
  );
  if ( EOK != result ) {
    return result;
  }
  // return success
  return EOK;
}
