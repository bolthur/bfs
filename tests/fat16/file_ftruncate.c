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

START_TEST( test_file_ftruncate_rofs ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // dummy so that not implemented test fails
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat16/" );
  ck_assert_ptr_null( mp );
  helper_unmount_test_image( "fat16", "/fat16/" );
}
END_TEST

START_TEST( test_file_ftruncate_rwfs ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // dummy so that not implemented test fails
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat16/" );
  ck_assert_ptr_null( mp );
  helper_unmount_test_image( "fat16", "/fat16/" );
}
END_TEST

START_TEST( test_file_ftruncate_extend_cluster ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // dummy so that not implemented test fails
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat16/" );
  ck_assert_ptr_null( mp );
  helper_unmount_test_image( "fat16", "/fat16/" );
}
END_TEST

START_TEST( test_file_ftruncate_shrink_cluster ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // dummy so that not implemented test fails
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat16/" );
  ck_assert_ptr_null( mp );
  helper_unmount_test_image( "fat16", "/fat16/" );
}
END_TEST

START_TEST( test_file_ftruncate_extend_size_only ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/truncate.txt", O_RDWR );
  ck_assert_int_eq( result, EOK );
  // save old size
  uint64_t old_size = file.fsize;
  // call for truncate
  result = fat_file_truncate( &file, old_size * 2 );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, old_size * 2 );
  // close file
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat16/hello/truncate.txt", O_RDONLY );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, old_size * 2 );
  // close file again
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}
END_TEST

START_TEST( test_file_ftruncate_shrink_size_only ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/truncate.txt", O_RDWR );
  ck_assert_int_eq( result, EOK );
  // save old size
  uint64_t old_size = file.fsize;
  // call for truncate
  result = fat_file_truncate( &file, old_size / 2 );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, old_size / 2 );
  // close file
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat16/hello/truncate.txt", O_RDONLY );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, old_size / 2 );
  // close file again
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}
END_TEST

Suite* fat16_suite_file_ftruncate( void ) {
  Suite* s = suite_create( "fat16_file_ftruncate" );
  TCase* tc_core = tcase_create( "fat16" );
  tcase_add_test( tc_core, test_file_ftruncate_rofs );
  tcase_add_test( tc_core, test_file_ftruncate_rwfs );
  tcase_add_test( tc_core, test_file_ftruncate_extend_cluster );
  tcase_add_test( tc_core, test_file_ftruncate_shrink_cluster );
  tcase_add_test( tc_core, test_file_ftruncate_extend_size_only );
  tcase_add_test( tc_core, test_file_ftruncate_shrink_size_only );
  suite_add_tcase( s, tc_core );
  return s;
}