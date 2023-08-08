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
#include <fat/iterator.h>
#include <fat/fs.h>
#include <fat/file.h>
#include <fat/stat.h>
#include "../_helper.hh"
#include "gtest/gtest.h"

TEST( fat32, stat_root_directory ) {
  helper_mount_fat_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // prepare stat structure
  struct stat st;
  memset( &st, 0, sizeof( st ) );
  // try to get stat
  int result = fat_stat( "/fat32/", &st );
  EXPECT_EQ( result, EOK );
  // open directory
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = fat_directory_open( &dir, "/fat32/" );
  EXPECT_EQ( result, EOK );
  // check mode
  EXPECT_EQ( st.st_mode, S_IFDIR );
  // close again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  helper_unmount_fat_test_image( "fat32", "/fat32/" );
}

TEST( fat32, stat_directory ) {
  helper_mount_fat_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // prepare stat structure
  struct stat st;
  memset( &st, 0, sizeof( st ) );
  // try to get stat
  int result = fat_stat( "/fat32/hello", &st );
  EXPECT_EQ( result, EOK );
  // open directory
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = fat_directory_open( &dir, "/fat32/hello/" );
  EXPECT_EQ( result, EOK );
  // check mode
  EXPECT_EQ( st.st_mode, S_IFDIR );
  // close again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  helper_unmount_fat_test_image( "fat32", "/fat32/" );
}

TEST( fat32, stat_file ) {
  helper_mount_fat_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // prepare stat structure
  struct stat st;
  memset( &st, 0, sizeof( st ) );
  // try to get stat
  int result = fat_stat( "/fat32/hello/world.txt", &st );
  EXPECT_EQ( result, EOK );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  result = fat_file_open( &file, "/fat32/hello/world.txt", "r" );
  EXPECT_EQ( result, EOK );
  // check mode
  EXPECT_EQ( st.st_mode, S_IFREG );
  EXPECT_GT( st.st_size, 0 );
  // close again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_fat_test_image( "fat32", "/fat32/" );
}
