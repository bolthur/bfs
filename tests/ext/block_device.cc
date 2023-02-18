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
#include <fcntl.h>
#include <common/errno.h>
#include <blockdev/tests/blockdev.h>
#include <ext/mountpoint.h>
#include <ext/fs.h>
#include "../_helper.hh"
#include "gtest/gtest.h"

TEST( ext, mount_null_block_device ) {
  // get block device
  common_blockdev_t* bdev = common_blockdev_get();
  EXPECT_TRUE( bdev );
  // set blockdev filename
  common_blockdev_set_fname( NULL );
  /// FIXME: SHOULD BE DONE WITHIN BLOCKDEV OPEN (?)
  bdev->part_offset = bdev->bdif->block_size;
  // register block device
  int result = common_blockdev_register_device( bdev, "ext2" );
  EXPECT_EQ( result, EOK );
  // mount device
  result = ext_mountpoint_mount( "ext2", "/ext2/", true );
  EXPECT_EQ( result, ENODATA );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 0U );
  // get fs
  ext_fs_t* fs = ( ext_fs_t* )bdev->fs;
  // assert fs and fs type
  EXPECT_FALSE( fs );
  // unmount
  result = ext_mountpoint_umount( "/ext2/" );
  EXPECT_EQ( result, ENODEV );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 0U );
  // unregister device
  result = common_blockdev_unregister_device( "ext2" );
  EXPECT_EQ( result, EOK );
}

TEST( ext, mount_invalid_block_device ) {
  // get block device
  common_blockdev_t* bdev = common_blockdev_get();
  EXPECT_TRUE( bdev );
  // set blockdev filename
  common_blockdev_set_fname( "ext.img" );
  /// FIXME: SHOULD BE DONE WITHIN BLOCKDEV OPEN (?)
  bdev->part_offset = bdev->bdif->block_size;
  // register block device
  int result = common_blockdev_register_device( bdev, "ext2" );
  EXPECT_EQ( result, EOK );
  // mount device
  result = ext_mountpoint_mount( "ext2", "/ext2/", true );
  EXPECT_EQ( result, EIO );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 0 );
  // get fs
  ext_fs_t* fs = ( ext_fs_t* )bdev->fs;
  // assert fs and fs type
  EXPECT_FALSE( fs );
  // unmount
  result = ext_mountpoint_umount( "/ext2/" );
  EXPECT_EQ( result, ENODEV );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 0 );
  // unregister device
  result = common_blockdev_unregister_device( "ext2" );
  EXPECT_EQ( result, EOK );
}
