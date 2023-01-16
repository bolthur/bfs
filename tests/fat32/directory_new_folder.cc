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
#include "../_helper.h"
#include "gtest/gtest.h"

TEST( fat32, directory_new_root_dir_folder ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // create new folder
  int result = fat_directory_make( "/fat32/wup/" );
  EXPECT_EQ( result, EOK );
  // load root dir
  result = fat_rootdir_open( mp, &dir );
  EXPECT_EQ( result, EOK );
  // try to find created folder
  result = fat_directory_entry_by_name( &dir, "wup" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // create new folder
  result = fat_directory_make( "/fat32/wup/" );
  EXPECT_EQ( result, EEXIST );
  helper_unmount_test_image( "fat32", "/fat32/" );
}

TEST( fat32, directory_new_dir_folder ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // create new folder
  int result = fat_directory_make( "/fat32/hello/foobar" );
  EXPECT_EQ( result, EOK );
  // load root dir
  result = fat_directory_open( &dir, "/fat32/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find created folder
  result = fat_directory_entry_by_name( &dir, "foobar" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // create new folder
  result = fat_directory_make( "/fat32/hello/foobar" );
  EXPECT_EQ( result, EEXIST );
  helper_unmount_test_image( "fat32", "/fat32/" );
}
