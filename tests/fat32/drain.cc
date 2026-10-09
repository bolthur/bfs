// Copyright (C) 2022 - 2026 bolthur project.
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
#include <fat/iterator.h>
#include <fat/fs.h>
#include <fat/file.h>
#include <fat/stat.h>
#include "../_helper.hh"
#include "gtest/gtest.h"

TEST( fat32, drain_create_write ) {
  helper_mount_fat_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // try to create file
  fat_file_t file = {};
  int result = fat_file_open2( &file, "/fat32/hello/drain.txt", O_RDWR | O_CREAT );
  EXPECT_EQ( result, EOK );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open /hello and check for asdf
  fat_directory_t dir = {};
  result = fat_directory_open( &dir, "/fat32/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find asdf
  result = fat_directory_entry_by_name( &dir, "drain.txt" );
  EXPECT_EQ( result, EOK );
  EXPECT_STREQ( dir.data->name, "drain.txt" );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount
  helper_unmount_fat_test_image( "fat32", "/fat32/" );
  // mount again
  helper_mount_fat_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // open /hello and check for asdf
  result = fat_directory_open( &dir, "/fat32/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find asdf
  result = fat_directory_entry_by_name( &dir, "drain.txt" );
  EXPECT_EQ( result, EOK );
  EXPECT_STREQ( dir.data->name, "drain.txt" );
  // umount
  helper_unmount_fat_test_image( "fat32", "/fat32/" );
}
