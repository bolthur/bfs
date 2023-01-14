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

START_TEST( test_directory_new_root_dir_folder ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat16/" );
  ck_assert_ptr_nonnull( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // create new folder
  int result = fat_directory_make( "/fat16/wup/" );
  ck_assert_int_eq( result, EOK );
  // load root dir
  result = fat_rootdir_open( mp, &dir );
  ck_assert_int_eq( result, EOK );
  // try to find created folder
  result = fat_directory_entry_by_name( &dir, "wup" );
  ck_assert_int_eq( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  ck_assert_int_eq( result, EOK );
  // create new folder
  result = fat_directory_make( "/fat16/wup/" );
  ck_assert_int_eq( result, EEXIST );
  helper_unmount_test_image( "fat16", "/fat16/" );
}
END_TEST

START_TEST( test_directory_new_dir_folder ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat16/" );
  ck_assert_ptr_nonnull( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // create new folder
  int result = fat_directory_make( "/fat16/hello/foobar" );
  ck_assert_int_eq( result, EOK );
  // load root dir
  result = fat_directory_open( &dir, "/fat16/hello/" );
  ck_assert_int_eq( result, EOK );
  // try to find created folder
  result = fat_directory_entry_by_name( &dir, "foobar" );
  ck_assert_int_eq( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  ck_assert_int_eq( result, EOK );
  // create new folder
  result = fat_directory_make( "/fat16/hello/foobar" );
  ck_assert_int_eq( result, EEXIST );
  helper_unmount_test_image( "fat16", "/fat16/" );
}
END_TEST

Suite* fat16_suite_directory_new_folder( void ) {
  Suite* s = suite_create( "fat16_directory_new_folder" );
  TCase* tc_core = tcase_create( "fat16" );
  tcase_add_test( tc_core, test_directory_new_root_dir_folder );
  tcase_add_test( tc_core, test_directory_new_dir_folder );
  suite_add_tcase( s, tc_core );
  return s;
}