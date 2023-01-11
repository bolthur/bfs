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

#include <string>
#include <cstring>
#include <fcntl.h>
#include <blockdev/tests/blockdev.h>
#include <common/mountpoint.h>
#include <common/blockdev.h>
#include <common/errno.h>
#include <fat/mountpoint.h>
#include <fat/structure.h>
#include <fat/directory.h>
#include <fat/type.h>
#include <fat/rootdir.h>
#include <fat/iterator.h>
#include <fat/fs.h>
#include <fat/file.h>
#include <gtest/gtest.h>
#include "_helper.hh"

/**
 * @brief Helper to mount test image
 *
 * @param read_only
 */
void helper_mount_test_image( bool read_only ) {
  // get block device
  common_blockdev_t* bdev = common_blockdev_get();
  EXPECT_TRUE( bdev );
  // set blockdev filename
  common_blockdev_set_fname( "../fat16.img" );
  /// FIXME: SHOULD BE DONE WITHIN BLOCKDEV OPEN (?)
  bdev->part_offset = bdev->bdif->block_size;
  // register block device
  int result = common_blockdev_register_device( bdev, "fat16" );
  EXPECT_EQ( result, EOK );
  // mount device
  result = fat_mountpoint_mount( "fat16", "/fat16/", read_only );
  EXPECT_EQ( result, EOK );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 1 );
  // get fs
  fat_fs_t* fs = static_cast< fat_fs_t* >( bdev->fs );
  // assert fs and fs type
  EXPECT_TRUE( fs );
  EXPECT_TRUE( fs->read_only == read_only );
  EXPECT_EQ( FAT_FAT16, fs->type );
}

/**
 * @brief Helper to unmount test image
 */
void helper_unmount_test_image( void ) {
  // get block device
  common_blockdev_t* bdev = common_blockdev_get();
  EXPECT_TRUE( bdev );
  // unmount
  int result = fat_mountpoint_umount( "/fat16/" );
  EXPECT_EQ( result, EOK );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 0 );
  // unregister device
  result = common_blockdev_unregister_device( "fat16" );
  EXPECT_EQ( result, EOK );
}
