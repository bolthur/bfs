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

TEST( fat16, file_write_append ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open2(
    &file,
    "/fat16/hello/file/write/append.txt",
    O_RDWR | O_APPEND
  );
  EXPECT_EQ( result, EOK );
  EXPECT_NE( file.cluster, 0 );
  EXPECT_NE( file.fsize, 0 );
  EXPECT_EQ( file.fsize, file.fpos );
  EXPECT_TRUE( file.mp );
  // allocate buffer for content
  char* buffer = ( char* )malloc( file.fsize + 1 );
  EXPECT_TRUE( buffer );
  memset( buffer, 0, file.fsize + 1 );
  // set to beginning
  result = fat_file_seek( &file, 0, SEEK_SET );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fpos, 0 );
  // read from file
  uint64_t read_count = 0;
  result = fat_file_read( &file, buffer, file.fsize, &read_count );
  buffer[ file.fsize ] = '\0';
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( read_count, file.fsize );
  EXPECT_STREQ( buffer, "hello world\n" );
  // write buffer again to file
  uint64_t write_count = 0;
  result = fat_file_write( &file, buffer, strlen( buffer ), &write_count );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( write_count, strlen( buffer ) );
  // allocate buffer for content
  free( buffer );
  buffer = ( char* )malloc( file.fsize + 1 );
  EXPECT_TRUE( buffer );
  memset( buffer, 0, file.fsize + 1 );
  // set to beginning
  result = fat_file_seek( &file, 0, SEEK_SET );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fpos, 0 );
  // read again from file
  file.fpos = read_count = 0;
  result = fat_file_read( &file, buffer, file.fsize, &read_count );
  buffer[ file.fsize ] = '\0';
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( read_count, file.fsize );
  EXPECT_STREQ( buffer, "hello world\nhello world\n" );
  // free again
  free( buffer );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}
