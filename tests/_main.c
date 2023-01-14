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

#include <stdlib.h>
#include "_main.h"

int main(void)
{
  int number_failed;
  SRunner* sr = srunner_create (blockdevice_suite_validate_test_driver());
  srunner_add_suite(sr, fat12_suite_directory_entry_by_name());
  srunner_add_suite(sr, fat12_suite_directory_iterator());
  srunner_add_suite(sr, fat12_suite_directory_new_folder());
  srunner_add_suite(sr, fat12_suite_directory_open());
  srunner_add_suite(sr, fat12_suite_directory_utils());
  srunner_add_suite(sr, fat12_suite_file_ftruncate());
  srunner_add_suite(sr, fat12_suite_file_open());
  srunner_add_suite(sr, fat12_suite_file_read());
  srunner_add_suite(sr, fat12_suite_invalid_block_device());
  srunner_add_suite(sr, fat12_suite_mount());
  srunner_add_suite(sr, fat12_suite_null_block_device());
  srunner_add_suite(sr, fat16_suite_directory_entry_by_name());
  srunner_add_suite(sr, fat16_suite_directory_iterator());
  srunner_add_suite(sr, fat16_suite_directory_new_folder());
  srunner_add_suite(sr, fat16_suite_directory_open());
  srunner_add_suite(sr, fat16_suite_directory_utils());
  srunner_add_suite(sr, fat16_suite_file_ftruncate());
  srunner_add_suite(sr, fat16_suite_file_open());
  srunner_add_suite(sr, fat16_suite_file_read());
  srunner_add_suite(sr, fat16_suite_invalid_block_device());
  srunner_add_suite(sr, fat16_suite_mount());
  srunner_add_suite(sr, fat16_suite_null_block_device());
  srunner_add_suite(sr, fat32_suite_directory_entry_by_name());
  srunner_add_suite(sr, fat32_suite_directory_iterator());
  srunner_add_suite(sr, fat32_suite_directory_new_folder());
  srunner_add_suite(sr, fat32_suite_directory_open());
  srunner_add_suite(sr, fat32_suite_directory_utils());
  srunner_add_suite(sr, fat32_suite_file_ftruncate());
  srunner_add_suite(sr, fat32_suite_file_open());
  srunner_add_suite(sr, fat32_suite_file_read());
  srunner_add_suite(sr, fat32_suite_invalid_block_device());
  srunner_add_suite(sr, fat32_suite_mount());
  srunner_add_suite(sr, fat32_suite_null_block_device());
  //srunner_set_fork_status(sr, CK_NOFORK);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
