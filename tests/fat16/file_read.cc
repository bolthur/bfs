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
#include <string>
#include <cstring>
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
#include <gtest/gtest.h>
#include "_helper.hh"

// Demonstrate some basic assertions.
TEST( FAT16, ReadFromFile ) {
  helper_mount_test_image( true );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open(
    &file,
    "/fat16/foobarlongfolder/foo/bar/hello.txt",
    "r"
  );
  EXPECT_EQ( result, EOK );
  EXPECT_NE( file.cluster, 0 );
  EXPECT_NE( file.fsize, 0 );
  EXPECT_TRUE( file.mp );
  // allocate buffer for content
  char* buffer = new char[ file.fsize + 1 ];
  EXPECT_TRUE( buffer );
  // read from file
  uint64_t read_count = 0;
  result = fat_file_read( &file, buffer, file.fsize, &read_count );
  buffer[ file.fsize ] = '\0';
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( read_count, file.fsize );
  EXPECT_STREQ( buffer, "world\n" );
  // free again
  delete[] buffer;
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image();
}
