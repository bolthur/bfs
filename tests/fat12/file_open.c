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

START_TEST( test_file_open_non_existant_file ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
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
  helper_unmount_test_image( "fat12", "/fat12/" );
}
END_TEST

START_TEST( test_file_open2_non_existant_file ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
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
  helper_unmount_test_image( "fat12", "/fat12/" );
}
END_TEST

START_TEST( test_file_open_existant_file ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
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
  helper_unmount_test_image( "fat12", "/fat12/" );
}
END_TEST

START_TEST( test_file_open2_existant_file ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
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
  helper_unmount_test_image( "fat12", "/fat12/" );
}
END_TEST

START_TEST( test_file_open_create_file_rofs ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to create file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat12/hello/asdf.txt", O_RDWR | O_CREAT );
  ck_assert_int_eq( result, EROFS );
  // open /hello and check for asdf
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = fat_directory_open( &dir, "/fat12/hello/" );
  ck_assert_int_eq( result, EOK );
  // try to find asdf
  result = fat_directory_entry_by_name( &dir, "asdf.txt" );
  ck_assert_int_eq( result, ENOENT );
  // close directory again
  result = fat_directory_close( &dir );
  ck_assert_int_eq( result, EOK );
  // umount
  helper_unmount_test_image( "fat12", "/fat12/" );
}
END_TEST

START_TEST( test_file_open_create_file_rwfs ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to create file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat12/hello/asdf.txt", O_RDWR | O_CREAT );
  ck_assert_int_eq( result, EOK );
  // open /hello and check for asdf
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = fat_directory_open( &dir, "/fat12/hello/" );
  ck_assert_int_eq( result, EOK );
  // try to find asdf
  result = fat_directory_entry_by_name( &dir, "asdf.txt" );
  ck_assert_int_eq( result, EOK );
  ck_assert_str_eq( dir.data->name, "asdf.txt" );
  // close directory again
  result = fat_directory_close( &dir );
  ck_assert_int_eq( result, EOK );
  // umount
  helper_unmount_test_image( "fat12", "/fat12/" );
}
END_TEST

Suite* fat12_suite_file_open( void ) {
  Suite* s = suite_create( "fat12_file_open" );
  TCase* tc_core = tcase_create( "fat12" );
  tcase_add_test( tc_core, test_file_open_non_existant_file );
  tcase_add_test( tc_core, test_file_open2_non_existant_file );
  tcase_add_test( tc_core, test_file_open_existant_file );
  tcase_add_test( tc_core, test_file_open2_existant_file );
  tcase_add_test( tc_core, test_file_open_create_file_rofs );
  tcase_add_test( tc_core, test_file_open_create_file_rwfs );
  suite_add_tcase( s, tc_core );
  return s;
}
