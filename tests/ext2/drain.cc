// Copyright (C) 2022 - 2026 bolthur project.
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

TEST( ext2, drain_create_write ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // try to create file
  ext_file_t file = {};
  int result = ext_file_open2( &file, "/ext2/hello/drain.txt", O_RDWR | O_CREAT );
  EXPECT_EQ( result, EOK );
  // close file again
  result = ext_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open /hello and check for asdf
  ext_directory_t dir = {};
  result = ext_directory_open( &dir, "/ext2/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find asdf
  result = ext_directory_entry_by_name( &dir, "drain.txt" );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( 0, strncmp( dir.entry->name, "drain.txt", dir.entry->name_len ) );
  // close directory again
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
  // mount again
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // open /hello and check for asdf
  result = ext_directory_open( &dir, "/ext2/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find asdf
  result = ext_directory_entry_by_name( &dir, "drain.txt" );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( 0, strncmp( dir.entry->name, "drain.txt", dir.entry->name_len ) );
  // umount
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}
