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
#include "../_helper.hh"
#include "gtest/gtest.h"

TEST( fat16, mount_null_block_device ) {
  // get block device
  common_blockdev_t* bdev = common_blockdev_get();
  EXPECT_TRUE( bdev );
  // set blockdev filename
  common_blockdev_set_fname( NULL );
  /// FIXME: SHOULD BE DONE WITHIN BLOCKDEV OPEN (?)
  bdev->part_offset = bdev->bdif->block_size;
  // register block device
  int result = common_blockdev_register_device( bdev, "fat16" );
  EXPECT_EQ( result, EOK );
  // mount device
  result = fat_mountpoint_mount( "fat16", "/fat16/", true );
  EXPECT_EQ( result, ENODATA );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 0U );
  // get fs
  fat_fs_t* fs = ( fat_fs_t* )bdev->fs;
  // assert fs and fs type
  EXPECT_FALSE( fs );
  // unmount
  result = fat_mountpoint_umount( "/fat16/" );
  EXPECT_EQ( result, ENODEV );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 0U );
  // unregister device
  result = common_blockdev_unregister_device( "fat16" );
  EXPECT_EQ( result, EOK );
}
