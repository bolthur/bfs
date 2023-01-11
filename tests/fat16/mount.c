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
#include <stdlib.h>
#include <string.h>
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

// Demonstrate some basic assertions.
START_TEST( test_mount_readonly ) {
  helper_mount_test_image( true );
  helper_unmount_test_image();
}
END_TEST

// Demonstrate some basic assertions.
START_TEST( test_mount_read_write ) {
  helper_mount_test_image( false );
  helper_unmount_test_image();
}
END_TEST

Suite* suite(void)
{
  Suite*s;
  TCase* tc_core;
  s = suite_create( "fat16" );
  tc_core = tcase_create( "Core" );
  tcase_add_test( tc_core, test_mount_readonly );
  tcase_add_test( tc_core, test_mount_read_write );
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
