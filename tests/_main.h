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

#include <check.h>

#ifndef _MAIN_H
#define _MAIN_H

Suite* blockdevice_suite_validate_test_driver(void);

Suite* fat12_suite_directory_entry_by_name(void);
Suite* fat12_suite_directory_iterator(void);
Suite* fat12_suite_directory_new_folder(void);
Suite* fat12_suite_directory_open(void);
Suite* fat12_suite_directory_utils(void);
Suite* fat12_suite_file_ftruncate(void);
Suite* fat12_suite_file_open(void);
Suite* fat12_suite_file_read(void);
Suite* fat12_suite_invalid_block_device(void);
Suite* fat12_suite_mount(void);
Suite* fat12_suite_null_block_device(void);

Suite* fat16_suite_directory_entry_by_name(void);
Suite* fat16_suite_directory_iterator(void);
Suite* fat16_suite_directory_new_folder(void);
Suite* fat16_suite_directory_open(void);
Suite* fat16_suite_directory_utils(void);
Suite* fat16_suite_file_ftruncate(void);
Suite* fat16_suite_file_open(void);
Suite* fat16_suite_file_read(void);
Suite* fat16_suite_invalid_block_device(void);
Suite* fat16_suite_mount(void);
Suite* fat16_suite_null_block_device(void);

Suite* fat32_suite_directory_entry_by_name(void);
Suite* fat32_suite_directory_iterator(void);
Suite* fat32_suite_directory_new_folder(void);
Suite* fat32_suite_directory_open(void);
Suite* fat32_suite_directory_utils(void);
Suite* fat32_suite_file_ftruncate(void);
Suite* fat32_suite_file_open(void);
Suite* fat32_suite_file_read(void);
Suite* fat32_suite_invalid_block_device(void);
Suite* fat32_suite_mount(void);
Suite* fat32_suite_null_block_device(void);

#endif
