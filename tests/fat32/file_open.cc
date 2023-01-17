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

TEST( fat32, file_open_non_existant_file ) {
  helper_mount_test_image( true, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open(
    &file,
    "/fat32/foobarlongfolder/foo/bar/holla.txt",
    "r"
  );
  EXPECT_EQ( result, ENOENT );
  EXPECT_EQ( file.cluster, 0 );
  EXPECT_FALSE( file.mp );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat32", "/fat32/" );
}

TEST( fat32, file_open2_non_existant_file ) {
  helper_mount_test_image( true, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open2(
    &file,
    "/fat32/foobarlongfolder/foo/bar/holla.txt",
    O_RDONLY
  );
  EXPECT_EQ( result, ENOENT );
  EXPECT_EQ( file.cluster, 0 );
  EXPECT_FALSE( file.mp );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat32", "/fat32/" );
}

TEST( fat32, file_open_existant_file ) {
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
  EXPECT_EQ( result, EOK );
  EXPECT_NE( file.cluster, 0 );
  EXPECT_NE( file.fsize, 0 );
  EXPECT_TRUE( file.mp );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat32", "/fat32/" );
}

TEST( fat32, file_open2_existant_file ) {
  helper_mount_test_image( true, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open2(
    &file,
    "/fat32/foobarlongfolder/foo/bar/hello.txt",
    O_RDONLY
  );
  EXPECT_EQ( result, EOK );
  EXPECT_NE( file.cluster, 0 );
  EXPECT_NE( file.fsize, 0 );
  EXPECT_TRUE( file.mp );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat32", "/fat32/" );
}

TEST( fat32, file_open_create_file_rofs ) {
  helper_mount_test_image( true, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // try to create file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat32/hello/asdf.txt", O_RDWR | O_CREAT );
  EXPECT_EQ( result, EROFS );
  // open /hello and check for asdf
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = fat_directory_open( &dir, "/fat32/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find asdf
  result = fat_directory_entry_by_name( &dir, "asdf.txt" );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount
  helper_unmount_test_image( "fat32", "/fat32/" );
}

TEST( fat32, file_open_create_file_rwfs ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // try to create file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat32/hello/asdf.txt", O_RDWR | O_CREAT );
  EXPECT_EQ( result, EOK );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open /hello and check for asdf
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = fat_directory_open( &dir, "/fat32/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find asdf
  result = fat_directory_entry_by_name( &dir, "asdf.txt" );
  EXPECT_EQ( result, EOK );
  EXPECT_STREQ( dir.data->name, "asdf.txt" );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount
  helper_unmount_test_image( "fat32", "/fat32/" );
}

TEST( fat32, file_open_truncate_file ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // try to create file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2(
    &file,
    "/fat32/hello/file/open/truncate.txt",
    O_RDWR
  );
  EXPECT_EQ( result, EOK );
  EXPECT_NE( file.fsize, 0 );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open with truncate
  result = fat_file_open2(
    &file,
    "/fat32/hello/file/open/truncate.txt",
    O_RDWR | O_TRUNC
  );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, 0 );
  // close file again
  result = fat_file_close( &file );
  // umount
  helper_unmount_test_image( "fat32", "/fat32/" );
}
