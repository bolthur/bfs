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

START_TEST( test_root_directory_read_dir_utils ) {
  helper_mount_test_image( true, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
  ck_assert_ptr_nonnull( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_rootdir_open( mp, &dir );
  ck_assert_int_eq( result, EOK );

  // get next entry
  result = fat_directory_next_entry( &dir );
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_nonnull( dir.entry );
  ck_assert_ptr_nonnull( dir.data );
  ck_assert_str_eq( "HELLO", dir.data->name );
  ck_assert_int_eq( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_directory_next_entry( &dir );
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_nonnull( dir.entry );
  ck_assert_ptr_nonnull( dir.data );
  ck_assert_str_eq( "WORLD.TXT", dir.data->name );

  // get next entry
  result = fat_directory_next_entry( &dir );
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_nonnull( dir.entry );
  ck_assert_ptr_nonnull( dir.data );
  ck_assert_str_eq( "LOREM.TXT", dir.data->name );

  // get next entry
  result = fat_directory_next_entry( &dir );
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_nonnull( dir.entry );
  ck_assert_ptr_nonnull( dir.data );
  ck_assert_str_eq( "foobarlongfolder", dir.data->name );
  ck_assert_int_eq( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // close directory
  result = fat_directory_close( &dir );
  ck_assert_int_eq( result, EOK );

  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

START_TEST( test_root_directory_read_dir_utils_rewind ) {
  helper_mount_test_image( true, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
  ck_assert_ptr_nonnull( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_rootdir_open( mp, &dir );
  ck_assert_int_eq( result, EOK );

  // get next entry
  result = fat_directory_next_entry( &dir );
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_nonnull( dir.entry );
  ck_assert_ptr_nonnull( dir.data );
  ck_assert_str_eq( "HELLO", dir.data->name );
  ck_assert_int_eq( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // rewind
  result = fat_directory_rewind( &dir );
  ck_assert_int_eq( result, EOK );

  // get next entry
  result = fat_directory_next_entry( &dir );
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_nonnull( dir.entry );
  ck_assert_ptr_nonnull( dir.data );
  ck_assert_str_eq( "HELLO", dir.data->name );
  ck_assert_int_eq( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // close directory
  result = fat_directory_close( &dir );
  ck_assert_int_eq( result, EOK );

  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

START_TEST( test_directory_get_by_name ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // open file
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open directory
  int result = fat_directory_open( &dir, "/fat32/hello/" );
  ck_assert_int_eq( result, EOK );
  // get entry
  result = fat_directory_entry_by_name( &dir, "truncate.txt" );
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_nonnull( dir.entry );
  ck_assert_ptr_nonnull( dir.data );
  ck_assert_str_eq( "TRUNCATE.TXT", dir.data->name );
  // close directory
  result = fat_directory_close( &dir );
  ck_assert_int_eq( result, EOK );
  // umount again
  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

Suite* fat32_suite_directory_utils(void) {
  Suite* s = suite_create( "fat32_directory_utils" );
  TCase* tc_core = tcase_create( "fat32" );
  tcase_add_test( tc_core, test_root_directory_read_dir_utils );
  tcase_add_test( tc_core, test_root_directory_read_dir_utils_rewind );
  tcase_add_test( tc_core, test_directory_get_by_name );
  suite_add_tcase( s, tc_core );
  return s;
}
