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
#include <ext/mountpoint.h>
#include <ext/structure.h>
#include <ext/directory.h>
#include <ext/type.h>
#include <ext/iterator.h>
#include <ext/fs.h>
#include <ext/file.h>
#include <ext/stat.h>
#include "../_helper.hh"
#include "gtest/gtest.h"

TEST( ext2, stat_root_directory ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // prepare stat structure
  struct stat st;
  memset( &st, 0, sizeof( st ) );
  // try to get stat
  int result = ext_stat( "/ext2/", &st );
  EXPECT_EQ( result, EOK );
  // open directory
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = ext_directory_open( &dir, "/ext2/" );
  EXPECT_EQ( result, EOK );
  // check inode
  EXPECT_EQ( st.st_ino, dir.inode_number );
  // close again
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, stat_directory ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  // prepare stat structure
  struct stat st;
  memset( &st, 0, sizeof( st ) );
  // try to get stat
  int result = ext_stat( "/ext2/hello", &st );
  EXPECT_EQ( result, EOK );
  // open directory
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = ext_directory_open( &dir, "/ext2/hello/" );
  EXPECT_EQ( result, EOK );
  // check inode
  EXPECT_EQ( st.st_ino, dir.inode_number );
  // close again
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, stat_file ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  // prepare stat structure
  struct stat st;
  memset( &st, 0, sizeof( st ) );
  // try to get stat
  int result = ext_stat( "/ext2/hello/world.txt", &st );
  EXPECT_EQ( result, EOK );
  // open file
  ext_file_t file;
  memset( &file, 0, sizeof( file ) );
  result = ext_file_open( &file, "/ext2/hello/world.txt", "r" );
  EXPECT_EQ( result, EOK );
  // check inode
  EXPECT_EQ( st.st_ino, file.inode_number );
  // close again
  result = ext_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}
