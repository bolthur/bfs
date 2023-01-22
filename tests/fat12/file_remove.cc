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

TEST( fat12, file_remove_rootdir_ro_fail ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_file_remove( "/fat12/fremove.txt" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat12/fremove.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, 0 );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, file_remove_rootdir_rw_success ) {
}

TEST( fat12, file_remove_rootdir_rw_longname ) {
}

TEST( fat12, file_remove_dir_ro_fail ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_file_remove( "/fat12/hello/file/remove.txt" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat12/hello/file/remove.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, 0 );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, file_remove_dir_rw_success ) {
}

TEST( fat12, file_remove_dir_rw_longname ) {
}
