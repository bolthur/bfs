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

START_TEST( test_directory_util_root_dir_get_existing_by_name ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  ck_assert_ptr_nonnull( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_rootdir_open( mp, &dir );
  ck_assert_int_eq( result, EOK );

  result = fat_directory_entry_by_name( &dir, "foobarlongfolder" );
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_nonnull( dir.entry );
  ck_assert_ptr_nonnull( dir.data );
  ck_assert_str_eq( "foobarlongfolder", dir.data->name );
  ck_assert_int_eq( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // close directory
  result = fat_directory_close( &dir );
  ck_assert_int_eq( result, EOK );

  helper_unmount_test_image( "fat12", "/fat12/" );
}
END_TEST

START_TEST( test_directory_util_dir_get_existing_by_name ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_directory_open( &dir, "/fat12/hello" );
  ck_assert_int_eq( result, EOK );

  result = fat_directory_entry_by_name( &dir, "world.txt" );
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_nonnull( dir.entry );
  ck_assert_ptr_nonnull( dir.data );
  ck_assert_str_eq( "WORLD.TXT", dir.data->name );

  // close directory
  result = fat_directory_close( &dir );
  ck_assert_int_eq( result, EOK );

  helper_unmount_test_image( "fat12", "/fat12/" );
}
END_TEST

START_TEST( test_directory_util_root_dir_get_non_existing_by_name ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  ck_assert_ptr_nonnull( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_rootdir_open( mp, &dir );
  ck_assert_int_eq( result, EOK );

  result = fat_directory_entry_by_name( &dir, "nonexistant" );
  ck_assert_int_eq( result, ENOENT );
  ck_assert_ptr_null( dir.entry );
  ck_assert_ptr_null( dir.data );

  // close directory
  result = fat_directory_close( &dir );
  ck_assert_int_eq( result, EOK );

  helper_unmount_test_image( "fat12", "/fat12/" );
}
END_TEST

START_TEST( test_directory_util_dir_get_non_existing_by_name ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_directory_open( &dir, "/fat12/hello" );
  ck_assert_int_eq( result, EOK );

  result = fat_directory_entry_by_name( &dir, "foo" );
  ck_assert_int_eq( result, ENOENT );
  ck_assert_ptr_null( dir.entry );
  ck_assert_ptr_null( dir.data );

  // close directory
  result = fat_directory_close( &dir );
  ck_assert_int_eq( result, EOK );

  helper_unmount_test_image( "fat12", "/fat12/" );
}
END_TEST

Suite* fat12_suite_directory_entry_by_name(void) {
  Suite* s = suite_create( "directory_entry_by_name" );
  TCase* tc_core = tcase_create( "fat12" );
  tcase_add_test( tc_core, test_directory_util_root_dir_get_existing_by_name );
  tcase_add_test( tc_core, test_directory_util_dir_get_existing_by_name );
  tcase_add_test( tc_core, test_directory_util_root_dir_get_non_existing_by_name );
  tcase_add_test( tc_core, test_directory_util_dir_get_non_existing_by_name );
  suite_add_tcase( s, tc_core );
  return s;
}
