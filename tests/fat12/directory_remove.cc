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

TEST( fat12, directory_remove_rootdir_ro_fail ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/remove/" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/remove/" );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( dir.entry_size, 0 );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_rootdir_rw_success ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/remove/" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get entry by name
  result = fat_directory_entry_by_name( &dir, "remove" );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_rootdir_rw_notempty ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/removefail/" );
  EXPECT_EQ( result, ENOTEMPTY );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get entry by name
  result = fat_directory_entry_by_name( &dir, "removefail" );
  EXPECT_EQ( result, EOK );
  EXPECT_STREQ( dir.data->name, "removefail" );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_rootdir_rw_longname ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/removelongname/" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get entry by name
  result = fat_directory_entry_by_name( &dir, "removelongname" );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_dir_ro_fail ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/hello/folder/remove/" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/hello/folder/remove/" );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( dir.entry_size, 0 );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_dir_rw_success ) {
}

TEST( fat12, directory_remove_dir_rw_notempty ) {
}

TEST( fat12, directory_remove_dir_rw_longname ) {
}
