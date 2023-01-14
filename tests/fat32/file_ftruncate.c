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
  helper_mount_test_image( true, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // dummy so that not implemented test fails
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
  ck_assert_ptr_null( mp );
  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

START_TEST( test_file_ftruncate_rwfs ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // dummy so that not implemented test fails
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
  ck_assert_ptr_null( mp );
  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

START_TEST( test_file_ftruncate_extend_cluster ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // dummy so that not implemented test fails
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
  ck_assert_ptr_null( mp );
  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

START_TEST( test_file_ftruncate_shrink_cluster ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // dummy so that not implemented test fails
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
  ck_assert_ptr_null( mp );
  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

START_TEST( test_file_ftruncate_change_size_only ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // dummy so that not implemented test fails
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
  ck_assert_ptr_null( mp );
  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

Suite* fat32_suite_file_ftruncate(void) {
  Suite* s = suite_create( "file_ftruncate" );
  TCase* tc_core = tcase_create( "fat32" );
  tcase_add_test( tc_core, test_file_ftruncate_rofs );
  tcase_add_test( tc_core, test_file_ftruncate_rwfs );
  tcase_add_test( tc_core, test_file_ftruncate_extend_cluster );
  tcase_add_test( tc_core, test_file_ftruncate_shrink_cluster );
  tcase_add_test( tc_core, test_file_ftruncate_change_size_only );
  suite_add_tcase( s, tc_core );
  return s;
}
