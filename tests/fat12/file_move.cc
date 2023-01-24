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
#include <fat/cluster.h>
#include <fat/file.h>
#include "../_helper.hh"
#include "gtest/gtest.h"

TEST( fat12, file_move_rootdir_ro_fail ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_file_move( "/fat12/fmove.txt", "/fat12/fmovelongname.txt" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat12/fmove.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, strlen("hello world\n") );
  // open base directory
  result = fat_file_open2( &file, "/fat12/fmovelongname.txt", O_RDONLY );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, file_move_rootdir_target_exist_fail ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_file_move( "/fat12/fmove.txt", "/fat12/world.txt" );
  EXPECT_EQ( result, EEXIST );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat12/fmove.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, strlen("hello world\n") );
  // open base directory
  result = fat_file_open2( &file, "/fat12/world.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, file_move_rootdir_source_not_exist_fail ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_file_move( "/fat12/fmoveinvalid.txt", "/fat12/world2.txt" );
  EXPECT_EQ( result, ENOENT );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat12/world.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, file_move_rootdir_rw_success ) {
}

TEST( fat12, file_move_dir_ro_fail ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_file_move( "/fat12/hello/file/fmove.txt", "/fat12/hello/file/fmovelongname.txt" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat12/hello/file/fmove.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, strlen("hello world\n") );
  // open base directory
  result = fat_file_open2( &file, "/fat12/hello/file/fmovelongname.txt", O_RDONLY );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, file_move_dir_target_exist_fail ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_file_move( "/fat12/hello/file/fmove.txt", "/fat12/hello/world.txt" );
  EXPECT_EQ( result, EEXIST );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat12/hello/file/fmove.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, strlen("hello world\n") );
  // open base directory
  result = fat_file_open2( &file, "/fat12/hello/world.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, file_move_dir_source_not_exist_fail ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_file_move( "/fat12/hello/file/fmoveinvalid.txt", "/fat12/world2.txt" );
  EXPECT_EQ( result, ENOENT );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat12/world.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, file_move_dir_rw_success ) {
}
