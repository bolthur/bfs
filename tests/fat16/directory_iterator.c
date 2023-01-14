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

START_TEST( test_directory_iterator_root_dir_read ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat16/" );
  ck_assert_ptr_nonnull( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_rootdir_open( mp, &dir );
  ck_assert_int_eq( result, EOK );

  fat_iterator_directory_t it;
  memset( &it, 0, sizeof( it ) );
  result = fat_iterator_directory_init(&it, &dir, 0);
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_nonnull( it.entry );
  ck_assert_str_eq( "HELLO", it.data->name );
  ck_assert_int_eq( it.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_iterator_directory_next( &it );
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_nonnull( it.entry );
  ck_assert_str_eq( "LOREM.TXT", it.data->name );

  // get next entry
  result = fat_iterator_directory_next( &it );
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_nonnull( it.entry );
  ck_assert_str_eq( "foobarlongfolder", it.data->name );

  // finish
  result = fat_iterator_directory_fini( &it );
  ck_assert_int_eq( result, EOK );
  ck_assert_ptr_null( it.entry );

  // close directory
  result = fat_directory_close( &dir );
  ck_assert_int_eq( result, EOK );

  helper_unmount_test_image( "fat16", "/fat16/" );
}
END_TEST

Suite* fat16_suite_directory_iterator(void) {
  Suite* s = suite_create( "directory_iterator" );
  TCase* tc_core = tcase_create( "fat16" );
  tcase_add_test( tc_core, test_directory_iterator_root_dir_read );
  suite_add_tcase( s, tc_core );
  return s;
}
