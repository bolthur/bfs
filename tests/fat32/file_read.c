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

START_TEST( test_file_read ) {
  helper_mount_test_image( true, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open(
    &file,
    "/fat32/foobarlongfolder/foo/bar/hello.txt",
    "r"
  );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_ne( file.cluster, 0 );
  ck_assert_uint_ne( file.fsize, 0 );
  ck_assert_ptr_nonnull( file.mp );
  // allocate buffer for content
  char* buffer = malloc( file.fsize + 1 );
  ck_assert_ptr_nonnull( buffer );
  // read from file
  uint64_t read_count = 0;
  result = fat_file_read( &file, buffer, file.fsize, &read_count );
  buffer[ file.fsize ] = '\0';
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( read_count, file.fsize );
  ck_assert_str_eq( buffer, "world\n" );
  // free again
  free( buffer );
  // close file
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

Suite* fat32_suite_file_read(void) {
  Suite* s = suite_create( "file_read" );
  TCase* tc_core = tcase_create( "fat32" );
  tcase_add_test( tc_core, test_file_read );
  suite_add_tcase( s, tc_core );
  return s;
}
