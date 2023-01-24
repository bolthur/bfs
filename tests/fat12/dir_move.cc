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

TEST( fat12, directory_move_rootdir_ro_fail ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_move( "/fat12/move", "/fat12/move2" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "move" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_move_rootdir_rw_notempty ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_move( "/fat12/movefail", "/fat12/move2" );
  EXPECT_EQ( result, ENOTEMPTY );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movefail" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "move2" );
  EXPECT_EQ( result, ENOENT );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_move_rootdir_rw_exist ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_move( "/fat12/move", "/fat12/movefail" );
  EXPECT_EQ( result, EEXIST );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "move" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_move_rootdir_source_not_exist_fail ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_move( "/fat12/moveasdf", "/fat12/moveasdf2" );
  EXPECT_EQ( result, ENOENT );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movefail" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_move_rootdir_rw_success ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_move( "/fat12/move", "/fat12/move2" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "MOVE" );
  EXPECT_EQ( result, ENOENT );
  // get by name
  result = fat_directory_entry_by_name( &dir, "move2" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // try to revert directory move
  result = fat_directory_move( "/fat12/move2", "/fat12/MOVE" );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "MOVE" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "move2" );
  EXPECT_EQ( result, ENOENT );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_move_dir_ro_fail ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_move( "/fat12/hello/folder/move", "/fat12/hello/folder/move2" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "move" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_move_dir_rw_notempty ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_move( "/fat12/hello/folder/movefail", "/fat12/hello/folder/move2" );
  EXPECT_EQ( result, ENOTEMPTY );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movefail" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "move2" );
  EXPECT_EQ( result, ENOENT );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_move_dir_rw_exist ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_move( "/fat12/hello/folder/move", "/fat12/hello/folder/movefail" );
  EXPECT_EQ( result, EEXIST );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movefail" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_move_dir_source_not_exist_fail ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_move( "/fat12/hello/folder/moveasdf", "/fat12/hello/folder/moveasdf2" );
  EXPECT_EQ( result, ENOENT );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movefail" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_move_dir_rw_success ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_move( "/fat12/hello/folder/move", "/fat12/hello/folder/move2" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "MOVE" );
  EXPECT_EQ( result, ENOENT );
  // get by name
  result = fat_directory_entry_by_name( &dir, "move2" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // try to revert directory move
  result = fat_directory_move( "/fat12/hello/folder/move2", "/fat12/hello/folder/MOVE" );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "MOVE" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "move2" );
  EXPECT_EQ( result, ENOENT );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}
