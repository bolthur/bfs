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
#include "../_helper.hh"
#include "gtest/gtest.h"

TEST( fat32, directory_util_root_dir_get_existing_by_name ) {
  helper_mount_test_image( true, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_rootdir_open( mp, &dir );
  EXPECT_EQ( result, EOK );

  result = fat_directory_entry_by_name( &dir, "foobarlongfolder" );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "foobarlongfolder", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_test_image( "fat32", "/fat32/" );
}

TEST( fat32, directory_util_dir_get_existing_by_name ) {
  helper_mount_test_image( true, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_directory_open( &dir, "/fat32/hello" );
  EXPECT_EQ( result, EOK );

  result = fat_directory_entry_by_name( &dir, "world.txt" );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "WORLD.TXT", dir.data->name );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_test_image( "fat32", "/fat32/" );
}

TEST( fat32, directory_util_root_dir_get_non_existing_by_name ) {
  helper_mount_test_image( true, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_rootdir_open( mp, &dir );
  EXPECT_EQ( result, EOK );

  result = fat_directory_entry_by_name( &dir, "nonexistant" );
  EXPECT_EQ( result, ENOENT );
  EXPECT_FALSE( dir.entry );
  EXPECT_FALSE( dir.data );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_test_image( "fat32", "/fat32/" );
}

TEST( fat32, directory_util_dir_get_non_existing_by_name ) {
  helper_mount_test_image( true, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_directory_open( &dir, "/fat32/hello" );
  EXPECT_EQ( result, EOK );

  result = fat_directory_entry_by_name( &dir, "foo" );
  EXPECT_EQ( result, ENOENT );
  EXPECT_FALSE( dir.entry );
  EXPECT_FALSE( dir.data );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_test_image( "fat32", "/fat32/" );
}
