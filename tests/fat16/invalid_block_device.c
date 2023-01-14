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
#include <string.h>
#include <stdlib.h>
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
#include <check.h>
#include "../_helper.h"

START_TEST( test_mount_invalid_block_device ) {
  // get block device
  common_blockdev_t* bdev = common_blockdev_get();
  ck_assert_ptr_nonnull( bdev );
  // set blockdev filename
  common_blockdev_set_fname( "fat.img" );
  /// FIXME: SHOULD BE DONE WITHIN BLOCKDEV OPEN (?)
  bdev->part_offset = bdev->bdif->block_size;
  // register block device
  int result = common_blockdev_register_device( bdev, "fat16" );
  ck_assert_int_eq( result, EOK );
  // mount device
  result = fat_mountpoint_mount( "fat16", "/fat16/", true );
  ck_assert_int_eq( result, EIO );
  // assert count
  ck_assert_uint_eq( bdev->bdif->reference_counter, 0 );
  // get fs
  fat_fs_t* fs = ( fat_fs_t* )bdev->fs;
  // assert fs and fs type
  ck_assert_ptr_null( fs );
  // unmount
  result = fat_mountpoint_umount( "/fat16/" );
  ck_assert_int_eq( result, ENODEV );
  // assert count
  ck_assert_uint_eq( bdev->bdif->reference_counter, 0 );
  // unregister device
  result = common_blockdev_unregister_device( "fat16" );
  ck_assert_int_eq( result, EOK );
}
END_TEST

Suite* fat16_suite_invalid_block_device( void )
{
  Suite* s = suite_create( "fat16_invalid_block_device" );
  TCase* tc_core = tcase_create( "fat16" );
  tcase_add_test( tc_core, test_mount_invalid_block_device );
  suite_add_tcase( s, tc_core );
  return s;
}
