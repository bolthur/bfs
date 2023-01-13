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
#include "_helper.h"

START_TEST( test_file_open_non_existant_file ) {
  helper_mount_test_image( true );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open(
    &file,
    "/fat12/foobarlongfolder/foo/bar/holla.txt",
    "r"
  );
  ck_assert_int_eq( result, ENOENT );
  ck_assert_uint_eq( file.cluster, 0 );
  ck_assert_ptr_null( file.mp );
  // close file
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  helper_unmount_test_image();
}
END_TEST

START_TEST( test_file_open2_non_existant_file ) {
  helper_mount_test_image( true );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open2(
    &file,
    "/fat12/foobarlongfolder/foo/bar/holla.txt",
    O_RDONLY
  );
  ck_assert_int_eq( result, ENOENT );
  ck_assert_uint_eq( file.cluster, 0 );
  ck_assert_ptr_null( file.mp );
  // close file
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  helper_unmount_test_image();
}
END_TEST

START_TEST( test_file_open_existant_file ) {
  helper_mount_test_image( true );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open(
    &file,
    "/fat12/foobarlongfolder/foo/bar/hello.txt",
    "r"
  );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_ne( file.cluster, 0 );
  ck_assert_uint_ne( file.fsize, 0 );
  ck_assert_ptr_nonnull( file.mp );
  // close file
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  helper_unmount_test_image();
}
END_TEST

START_TEST( test_file_open2_existant_file ) {
  helper_mount_test_image( true );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open2(
    &file,
    "/fat12/foobarlongfolder/foo/bar/hello.txt",
    O_RDONLY
  );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_ne( file.cluster, 0 );
  ck_assert_uint_ne( file.fsize, 0 );
  ck_assert_ptr_nonnull( file.mp );
  // close file
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  helper_unmount_test_image();
}
END_TEST

START_TEST( test_file_open_create_file_rofs ) {
  helper_mount_test_image( true );
  // dummy so that not implemented test fails
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  ck_assert_ptr_null( mp );
  helper_unmount_test_image();
}
END_TEST

START_TEST( test_file_open_create_file_rwfs ) {
  helper_mount_test_image( false );
  // dummy so that not implemented test fails
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  ck_assert_ptr_null( mp );
  helper_unmount_test_image();
}
END_TEST

Suite* suite(void)
{
  Suite*s;
  TCase* tc_core;
  s = suite_create( "fat12" );
  tc_core = tcase_create( "Core" );
  tcase_add_test( tc_core, test_file_open_non_existant_file );
  tcase_add_test( tc_core, test_file_open2_non_existant_file );
  tcase_add_test( tc_core, test_file_open_existant_file );
  tcase_add_test( tc_core, test_file_open2_existant_file );
  tcase_add_test( tc_core, test_file_open_create_file_rofs );
  tcase_add_test( tc_core, test_file_open_create_file_rwfs );
  suite_add_tcase( s, tc_core );
  return s;
}

int main(void)
{
  int number_failed;
  Suite *s = suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
