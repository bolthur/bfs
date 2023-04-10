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
#include "../_helper.hh"
#include "gtest/gtest.h"

TEST( ext2, directory_open_sub_directory ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = ext_directory_open( &dir, "/ext2/foobarlongfolder/foo/bar" );
  EXPECT_EQ( result, EOK );

  ext_iterator_directory_t it;
  memset( &it, 0, sizeof( it ) );
  result = ext_iterator_directory_init(&it, &dir, 0);
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.name );
  EXPECT_STREQ( ".", it.name );

  // finish
  result = ext_iterator_directory_fini( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_FALSE( it.name );

  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, root_directory_read_dir_utils ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/ext2/" );
  EXPECT_TRUE( mp );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = ext_directory_open( &dir, "/ext2/" );
  EXPECT_EQ( result, EOK );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( ".", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "..", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "lost+found", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "fmove.txt", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "fmovelongname.txt", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "foobarlongfolder", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "fremove.txt", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "fremovelongname.txt", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "hello", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "lorem.txt", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "move", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "movefail", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "movelongname", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "remove", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "removefail", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "removelongname", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "world.txt", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, root_directory_read_dir_utils_rewind ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/ext2/" );
  EXPECT_TRUE( mp );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = ext_directory_open( &dir, "/ext2/" );
  EXPECT_EQ( result, EOK );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( ".", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "..", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "lost+found", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // rewind
  result = ext_directory_rewind( &dir );
  EXPECT_EQ( result, EOK );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( ".", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_get_by_name ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // open file
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open directory
  int result = ext_directory_open( &dir, "/ext2/hello/" );
  EXPECT_EQ( result, EOK );
  // get entry
  result = ext_directory_entry_by_name( &dir, "truncate.txt" );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "truncate.txt", dir.name );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );
  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount again
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_iterator_root_dir_read ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_util_root_dir_get_existing_by_name ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_util_dir_get_existing_by_name ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_util_root_dir_get_non_existing_by_name ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_util_dir_get_non_existing_by_name ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_new_root_dir_folder ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_new_dir_folder ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_new_root_dir_folder_long ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_new_dir_folder_long ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_remove_rootdir_ro_fail ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_remove_rootdir_rw_success ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_remove_rootdir_rw_notempty ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_remove_rootdir_rw_longname ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_remove_dir_ro_fail ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_remove_dir_rw_success ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_remove_dir_rw_notempty ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_remove_dir_rw_longname ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_rootdir_ro_fail ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_rootdir_rw_notempty ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_rootdir_rw_exist ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_rootdir_source_not_exist_fail ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_rootdir_rw_short_name_success ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_rootdir_rw_long_name_success ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_ro_fail ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_rw_notempty ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_rw_exist ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_source_not_exist_fail ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_rw_short_name_success ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_rw_long_name_success ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_rw_short_name_cluster_check ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_to_dir_rw_long_name_cluster_check ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}
