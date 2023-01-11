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
TEST( FAT16, OpenSubDirectory ) {
  helper_mount_test_image( true );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat16/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_directory_open( &dir, "/fat16/foobarlongfolder/foo/bar" );
  EXPECT_EQ( result, EOK );

  fat_iterator_directory_t it;
  memset( &it, 0, sizeof( it ) );
  result = fat_iterator_directory_init(&it, &dir, 0);
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "HELLO.TXT", it.data->name );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  // finish
  result = fat_iterator_directory_fini( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_FALSE( it.entry );
  helper_unmount_test_image();
}
