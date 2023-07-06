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
#include <string.h>
#include <common/errno.h> // IWYU pragma: keep
#include <common/blockdev.h>
#include <ext/superblock.h>
#include <ext/blockgroup.h>
#include <ext/block.h>
#include <ext/inode.h>
#include <ext/indirection.h>
#include <ext/fs.h>
#include <ext/structure.h>
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
  uint8_t* buffer = malloc( ( size_t )block_size );
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
 * @brief Method to write an inode
 *
 * @param fs
 * @param number
 * @param inode
 * @return int
 */
BFSEXT_NO_EXPORT int ext_inode_write_inode(
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
  // read data
  result = common_blockdev_bytes_write(
    fs->bdev,
    descriptor.bg_inode_table * block_size + inode_offset,
    inode,
    sizeof( *inode )
  );
  if ( EOK != result ) {
    return ENOMEM;
  }
  return EOK;
}

/**
 * @brief Get block offset
 *
 * @param fs
 * @param inode
 * @param block_no
 * @param offset
 * @param allocate
 * @return int
 */
BFSEXT_NO_EXPORT int ext_inode_get_block_offset(
  ext_fs_t* fs,
  ext_structure_inode_t* inode,
  uint64_t block_no,
  uint64_t* offset,
  inode_allocate_t allocate
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
  if ( ! block_nr && INODE_ALLOC == allocate ) {
    uint64_t block;
    result = ext_block_allocate( fs, &block );
    if ( EOK != result ) {
      return result;
    }
    block_nr = ( uint32_t )block;
    inode->i_block[ direct_block ] = block_nr;
    inode->i_blocks++;
  }
  // allocate space for table
  uint8_t* table = malloc( ( size_t )block_size );
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
    if ( ! block_nr && INODE_ALLOC == allocate ) {
      // allocate block
      uint64_t block;
      result = ext_block_allocate( fs, &block );
      if ( EOK != result ) {
        return result;
      }
      // push to table
      (( uint32_t* )table)[ off ] = ( uint32_t )block;
      inode->i_blocks++;
      // write back table
      result = common_blockdev_bytes_write(
        fs->bdev,
        block_nr * block_size,
        table,
        block_size
      );
    }
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
  uint8_t* tmp = malloc( ( size_t )block_size );
  if ( ! tmp ) {
    return ENOMEM;
  }
  // loop and load blocks
  for ( uint64_t idx = 0; idx < count; idx++ ) {
    uint64_t offset;
    result = ext_inode_get_block_offset( fs, inode, block_no + idx, &offset, INODE_NO_ALLOC );
    if ( EOK != result ) {
      free( tmp );
      return result;
    }
    // handle sparse file
    if ( 0 == offset ) {
      memset( buffer + block_size * idx, 0, ( size_t )block_size );
      continue;
    }
    // load block
    result = common_blockdev_bytes_read( fs->bdev, offset, tmp, block_size );
    if ( EOK != result ) {
      free( tmp );
      return result;
    }
    // copy over
    memcpy( buffer + block_size * idx, tmp, ( size_t )block_size );
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
  uint8_t* local_buffer = malloc( ( size_t )block_size );
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
    memcpy( buffer, local_buffer + offset, ( size_t )copy_size );
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
    memcpy( buffer + length - copy_size, local_buffer, ( size_t )copy_size );
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

/**
 * @brief Write block
 *
 * @param fs
 * @param inode
 * @param block_no
 * @param buffer
 * @param count
 * @return int
 */
BFSEXT_NO_EXPORT int ext_inode_write_block(
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
  // loop and load blocks
  for ( uint64_t idx = 0; idx < count; idx++ ) {
    uint64_t offset;
    result = ext_inode_get_block_offset( fs, inode, block_no + idx, &offset, INODE_ALLOC );
    if ( EOK != result ) {
      return result;
    }
    // handle sparse file
    if ( 0 == offset ) {
      continue;
    }
    // load block
    result = common_blockdev_bytes_write(
      fs->bdev, offset, buffer + block_size * idx, block_size );
    if ( EOK != result ) {
      return result;
    }
  }
  return EOK;
}

/**
 * @brief Write data
 *
 * @param fs
 * @param inode
 * @param start
 * @param length
 * @param buffer
 * @return int
 */
BFSEXT_NO_EXPORT int ext_inode_write_data(
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
  uint64_t req_length = length;
  // allocate a local buffer
  uint8_t* local_buffer = malloc( ( size_t )block_size );
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
    memcpy( local_buffer + offset, buffer, ( size_t )copy_size );
    // write stuff
    result = ext_inode_write_block( fs, inode, start_block, local_buffer, 1 );
    if ( EOK != result ) {
      free( local_buffer );
      return result;
    }
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
    memcpy( local_buffer, buffer + length - copy_size, ( size_t )copy_size );
    // write stuff
    result = ext_inode_write_block( fs, inode, start_block, local_buffer, 1 );
    if ( EOK != result ) {
      free( local_buffer );
      return result;
    }
    // decrement length and end block
    length -= copy_size;
    end_block--;
  }
  // calculate block count
  uint64_t block_count = end_block - start_block + 1;
  // write remaining blocks
  if ( block_count > 0 ) {
    result = ext_inode_write_block( fs, inode, start_block, buffer, block_count );
    if ( EOK != result ) {
      free( local_buffer );
      return result;
    }
  }
  if ( ( uint32_t )( start + req_length ) > inode->i_size ) {
    inode->i_size = ( uint32_t )( start + req_length );
  }
  // free local buffer and return ok
  free( local_buffer );
  return EOK;
}

/**
 * @brief Allocate new inode
 *
 * @param fs
 * @param inode
 * @param number
 * @return int
 */
BFSEXT_NO_EXPORT int ext_inode_allocate(
  ext_fs_t* fs,
  ext_structure_inode_t* inode,
  uint64_t* inode_number
) {
  // validate parameter
  if ( ! fs || ! inode || ! inode_number ) {
    return EINVAL;
  }

  uint64_t number = 0;
  uint64_t bgnumber;
  uint64_t bgcount;
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // get block group number
  result = ext_superblock_blockgroup_count( fs, &bgcount );
  if ( EOK != result ) {
    return result;
  }
  // allocate space for bitmap
  uint8_t* bitmap = malloc( ( size_t )block_size );
  if ( ! bitmap ) {
    return ENOMEM;
  }

  for ( bgnumber = 0; bgnumber < bgcount && 0 == number; bgnumber++ ) {
    // read block group descriptor
    ext_structure_block_group_descriptor_t bgd;
    memset( &bgd, 0, sizeof( bgd ) );
    result = ext_blockgroup_read( fs, bgnumber, &bgd );
    if ( EOK != result ) {
      free( bitmap );
      return result;
    }
    // read bitmap
    memset( bitmap, 0, ( size_t )block_size );
    result = ext_block_read_bitmap( fs, &bgd, ( uint32_t* )bitmap );
    if ( EOK != result ) {
      free( bitmap );
      return result;
    }
    for (
      uint64_t i = 0;
      i < ( fs->superblock.s_inodes_per_group + 31 ) / 32 && 0 == number;
      i++
    ) {
      // skip if inode is not empty
      if ( ( ( uint32_t* )bitmap )[i] == ~0U ) {
        continue;
      }
      for ( uint64_t j = 0; j < 32 && 0 == number; j++ ) {
        if (
          ( 0 == ( ( ( uint32_t* )bitmap )[ i ] & ( 1U << j ) ) )
          && ( i * 32 + j <= fs->superblock.s_inodes_per_group )
        ) {
          // mark as used in bitmap
          ( ( uint32_t* )bitmap )[ i ] |= ( 1U << j );
          // write back bitmap
          result = ext_block_write_bitmap( fs, &bgd, ( uint32_t* )bitmap );
          if ( EOK != result ) {
            free( bitmap );
            return result;
          }
          // update suberblock
          fs->superblock.s_free_inodes_count--;
          result = ext_superblock_write( fs, &fs->superblock );
          if ( EOK != result ) {
            free( bitmap );
            return result;
          }
          // update block group
          bgd.bg_free_inodes_count--;
          result = ext_blockgroup_write( fs, bgnumber, &bgd );
          if ( EOK != result ) {
            free( bitmap );
            return result;
          }
          // set block group number
          number = (bgnumber * fs->superblock.s_inodes_per_group) + ( i * 32 ) + j;
        }
      }
    }
  }
  // free bitmap
  free( bitmap );
  // handle nothing found
  if ( 0 == number ) {
    return EINVAL;
  }
  // read inode
  result = ext_inode_read_inode( fs, number + 1, inode );
  if ( EOK != result ) {
    return result;
  }
  memset( inode, 0, sizeof( ext_structure_inode_t ) );
  *inode_number = number;
  // return success
  return EOK;
}

/**
 * @brief deallocate inode blocks recursive
 *
 * @param fs
 * @param table_block
 * @param level
 * @return int
 */
BFSEXT_NO_EXPORT int ext_inode_deallocate_block_recursive(
  ext_fs_t* fs,
  uint64_t table_block,
  uint64_t level
) {
  if ( ! fs ) {
    return EINVAL;
  }
  // gather block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // allocate space for table
  uint8_t* table = malloc( ( size_t )block_size );
  if ( ! table ) {
    return ENOMEM;
  }
  // read block data
  result = common_blockdev_bytes_read(
    fs->bdev, table_block * block_size, table, block_size );
  if ( EOK != result ) {
    free( table );
    return result;
  }
  // loop until block size
  for ( uint64_t idx = 0; idx < block_size / 4; idx++ ) {
    if ( ( ( uint32_t* )table )[ idx ] ) {
      if ( level ) {
        result = ext_inode_deallocate_block_recursive(
          fs,
          ( ( uint32_t* )table )[ idx ],
          level - 1
        );
        if ( EOK != result ) {
          free( table );
          return result;
        }
      }
      result = ext_block_deallocate( fs, ( ( uint32_t* )table )[ idx ] );
      if ( EOK != result ) {
        free( table );
        return result;
      }
    }
  }
  // write back block data
  result = common_blockdev_bytes_write(
    fs->bdev, table_block * block_size, table, block_size );
  if ( EOK != result ) {
    free( table );
    return result;
  }
  free( table );
  return EOK;
}

/**
 * @brief Helper to deallocate an inode
 *
 * @param fs
 * @param inode
 * @param number
 * @return int
 */
BFSEXT_NO_EXPORT int ext_inode_deallocate(
  ext_fs_t* fs,
  ext_structure_inode_t* inode,
  uint64_t number
) {
  // handle invalid data
  if ( ! fs || ! inode || ! number ) {
    return EINVAL;
  }
  // FIXME: set correct deletion time
  inode->i_dtime = 1;
  // get block size
  uint64_t block_size;
  int result = ext_superblock_block_size( fs, &block_size );
  if ( EOK != result ) {
    return result;
  }
  // translate to local number
  uint64_t local_inode_number;
  result = ext_inode_get_local_inode( fs, number, &local_inode_number );
  if ( EOK != result ) {
    return result;
  }
  // read block group number
  uint64_t bgnum = local_inode_number / fs->superblock.s_inodes_per_group;
  uint64_t num = local_inode_number % fs->superblock.s_inodes_per_group;
  // read block group descriptor
  ext_structure_block_group_descriptor_t bgd;
  result = ext_blockgroup_read( fs, bgnum, &bgd );
  if ( EOK != result ) {
    return result;
  }
  uint8_t* bitmap = malloc( ( size_t )block_size );
  if ( ! bitmap ) {
    return ENOMEM;
  }
  // read bitmap
  memset( bitmap, 0, ( size_t )block_size );
  result = ext_block_read_bitmap( fs, &bgd, ( uint32_t* )bitmap );
  if ( EOK != result ) {
    free( bitmap );
    return result;
  }
  // free in bitmap
  ( ( uint32_t* )bitmap )[num / 32] &= ~(1U << (num % 32));
  // write back bitmap
  result = ext_block_write_bitmap( fs, &bgd, ( uint32_t* )bitmap );
  if ( EOK != result ) {
    free( bitmap );
    return result;
  }
  // free bitmap again
  free( bitmap );
  // update superblock
  fs->superblock.s_free_inodes_count++;
  result = ext_superblock_write( fs, &fs->superblock );
  if ( EOK != result ) {
    return result;
  }
  // update block group descriptor
  bgd.bg_free_inodes_count++;
  result = ext_blockgroup_write( fs, bgnum, &bgd );
  if ( EOK != result ) {
    return result;
  }
  // handle no blocks
  if ( 0 < inode->i_blocks ) {
    return EOK;
  }
  // free all blocks
  for ( uint64_t idx = 0; idx < 15; idx++ ) {
    // skip null blocks
    if ( 0 >= inode->i_block[ idx ] ) {
      continue;
    }
    // handle indirection stuff
    if ( idx >= 12 ) {
      result = ext_inode_deallocate_block_recursive(
        fs, inode->i_block[ idx ], idx - 12
      );
      if ( EOK != result ) {
        return result;
      }
    }
    result = ext_block_deallocate( fs, inode->i_block[ idx ] );
    if ( EOK != result ) {
      return result;
    }
  }
  // return success
  return EOK;
}
