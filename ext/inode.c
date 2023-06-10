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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <common/errno.h>
#include <ext/superblock.h>
#include <ext/blockgroup.h>
#include <ext/inode.h>
#include <ext/indirection.h>
#include <ext/bfsext_export.h>

/**
 * @brief Helper to get local inode index
 *
 * @param fs
 * @param inode
 * @param result
 * @return int
 */
BFSEXT_NO_EXPORT int ext_inode_get_local_inode(
  ext_fs_t* fs,
  uint64_t inode,
  uint64_t* result
) {
  // validate parameter
  if ( ! fs || ! result ) {
    return EINVAL;
  }
  // calculate block group and push it to result
  *result = ( inode - 1 );
  // return success
  return EOK;
}

/**
 * @brief Method to read an inode
 *
 * @param fs
 * @param number
 * @param inode
 * @return int
 */
BFSEXT_NO_EXPORT int ext_inode_read_inode(
  ext_fs_t* fs,
  uint64_t number,
  ext_structure_inode_t* inode
) {
  // validate
  if ( ! fs || ! inode ) {
    return EINVAL;
  }
  // get inode size
  uint64_t inode_size;
  int result = ext_superblock_inode_size( fs, &inode_size );
  if ( EOK != result ) {
    return result;
  }
  // get block size
  uint64_t block_size;
  result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // get internal inode
  uint64_t number_local;
  result = ext_inode_get_local_inode( fs, number, &number_local );
  if ( EOK != result ) {
    return result;
  }
  // read block group descriptor
  ext_structure_block_group_descriptor_t descriptor;
  result = ext_blockgroup_read( fs, number_local / fs->superblock.s_inodes_per_group, &descriptor );
  if ( EOK != result ) {
    return result;
  }
  // calculate offset
  uint64_t inode_offset = inode_size * ( number_local % fs->superblock.s_inodes_per_group );
  // allocate buffer
  uint8_t* buffer = malloc( block_size );
  if ( ! buffer ) {
    return ENOMEM;
  }
  // read data
  result = common_blockdev_bytes_read(
    fs->bdev,
    descriptor.bg_inode_table * block_size + inode_offset,
    buffer,
    block_size
  );
  if ( EOK != result ) {
    free( buffer );
    return ENOMEM;
  }
  // copy over data
  memcpy( inode, buffer, sizeof( *inode ) );
  // free data
  free( buffer );
  return EOK;
}

/**
 * @brief Get block offset
 *
 * @param fs
 * @param inode
 * @param block_no
 * @param offset
 * @return BFSEXT_NO_EXPORT
 */
BFSEXT_NO_EXPORT int ext_inode_get_block_offset(
  ext_fs_t* fs,
  ext_structure_inode_t* inode,
  uint64_t block_no,
  uint64_t* offset
) {
  // validate parameter
  if ( ! fs || ! inode || ! offset ) {
    return EINVAL;
  }
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // get indirection level
  uint64_t level;
  uint64_t direct_block;
  uint64_t indirect_block;
  result = ext_indirection_level( fs, block_no, &direct_block, &indirect_block, &level );
  if ( EOK != result ) {
    return result;
  }
  // read direct block
  uint32_t block_nr = inode->i_block[ direct_block ];
  // allocate space for table
  uint8_t* table = malloc( block_size );
  if ( ! table ) {
    return ENOMEM;
  }
  // loop through levels
  for ( uint64_t idx = 0; idx < level && block_nr; idx++ ) {
    // read block data
    result = common_blockdev_bytes_read(
      fs->bdev, block_nr * block_size, table, block_size );
    if ( EOK != result ) {
      free( table );
      return result;
    }
    // adjust indirect block
    uint64_t pow = (level - idx == 3 ? ((block_size / 4) * (block_size / 4)) :
      (level - idx == 2 ? (block_size / 4) : 1));
    uint64_t off = indirect_block / pow;
    indirect_block %= pow;
    // overwrite block number
    block_nr = (( uint32_t* )table)[ off ];
  }
  // free table
  free( table );
  *offset = block_size * block_nr;
  return EOK;
}

/**
 * @brief Read block
 *
 * @param fs
 * @param inode
 * @param block_no
 * @param buffer
 * @param count
 * @return BFSEXT_NO_EXPORT
 */
BFSEXT_NO_EXPORT int ext_inode_read_block(
  ext_fs_t* fs,
  ext_structure_inode_t* inode,
  uint64_t block_no,
  uint8_t* buffer,
  uint64_t count
) {
  // validate
  if ( ! fs || ! inode || ! buffer ) {
    return EINVAL;
  }
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // allocate buffer
  char* tmp = malloc( block_size );
  if ( ! tmp ) {
    return ENOMEM;
  }
  // loop and load blocks
  for ( uint64_t idx = 0; idx < count; idx++ ) {
    uint64_t offset;
    result = ext_inode_get_block_offset( fs, inode, block_no + idx, &offset );
    if ( EOK != result ) {
      free( tmp );
      return result;
    }
    // handle sparse file
    if ( 0 == offset ) {
      memset( buffer + block_size * idx, 0, block_size );
      continue;
    }
    // load block
    result = common_blockdev_bytes_read( fs->bdev, offset, tmp, block_size );
    if ( EOK != result ) {
      free( tmp );
      return result;
    }
    // copy over
    memcpy( buffer + block_size * idx, tmp, block_size );
  }
  // free tmp and return success
  free( tmp );
  return EOK;
}

/**
 * @brief Read inode data
 *
 * @param fs
 * @param inode
 * @param start
 * @param length
 * @param buffer
 * @return BFSEXT_NO_EXPORT
 */
BFSEXT_NO_EXPORT int ext_inode_read_data(
  ext_fs_t* fs,
  ext_structure_inode_t* inode,
  uint64_t start,
  uint64_t length,
  uint8_t* buffer
) {
  // validate parameter
  if ( ! fs || ! inode || ! buffer || ! length ) {
    return EINVAL;
  }
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // determine start and end block
  uint64_t start_block = start / block_size;
  uint64_t end_block = ( start + length - 1 ) / block_size;
  // allocate a local buffer
  uint8_t* local_buffer = malloc( block_size );
  if ( ! local_buffer ) {
    return ENOMEM;
  }

  // handle unaligned start
  if ( start % block_size ) {
    uint64_t offset = start % block_size;
    // read to local buffer
    result = ext_inode_read_block( fs, inode, start_block, local_buffer, 1 );
    if ( EOK != result ) {
      free( local_buffer );
      return result;
    }
    // calculate copy offset
    uint64_t copy_size = block_size - offset;
    if ( length < copy_size ) {
      copy_size = length;
    }
    // copy
    memcpy( buffer, local_buffer + offset, copy_size );
    // decrement length and increment buffer
    length -= copy_size;
    buffer += copy_size;
    start_block++;
  }
  // handle unaligned end
  if ( length % block_size ) {
    // determine copy size
    uint64_t copy_size = length % block_size;
    // read to local buffer
    result = ext_inode_read_block( fs, inode, end_block, local_buffer, 1 );
    if ( EOK != result ) {
      free( local_buffer );
      return result;
    }
    // copy over
    memcpy( buffer + length - copy_size, local_buffer, copy_size );
    // decrement length and end block
    length -= copy_size;
    end_block--;
  }
  // calculate block count
  uint64_t block_count = end_block - start_block + 1;
  // load remaining blocks
  // read to local buffer
  result = ext_inode_read_block( fs, inode, start_block, buffer, block_count );
  if ( EOK != result ) {
    free( local_buffer );
    return result;
  }
  // free local buffer and return ok
  free( local_buffer );
  return EOK;
}
