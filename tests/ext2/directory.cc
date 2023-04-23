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
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( ".", it.entry->name, it.entry->name_len ) );

  // finish
  result = ext_iterator_directory_fini( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_FALSE( it.entry );

  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_read_dir_utils ) {
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
  EXPECT_EQ( 0, strncmp( ".", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "..", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "lost+found", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "fmove.txt", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "fmovelongname.txt", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "foobarlongfolder", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "fremove.txt", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "fremovelongname.txt", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "hello", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "lorem.txt", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "move", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "movefail", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "movelongname", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "remove", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "removefail", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "removelongname", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "world.txt", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_read_dir_utils_rewind ) {
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
  EXPECT_EQ( 0, strncmp( ".", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "..", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( "lost+found", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // rewind
  result = ext_directory_rewind( &dir );
  EXPECT_EQ( result, EOK );

  // get next entry
  result = ext_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_EQ( 0, strncmp( ".", dir.entry->name, dir.entry->name_len ) );
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
  EXPECT_EQ( 0, strncmp( "truncate.txt", dir.entry->name, dir.entry->name_len ) );
  EXPECT_EQ( dir.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );
  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount again
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_iterator_dir_read ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/ext2/" );
  EXPECT_TRUE( mp );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = ext_directory_open( &dir, "/ext2/" );
  EXPECT_EQ( result, EOK );

  ext_iterator_directory_t it;
  memset( &it, 0, sizeof( it ) );
  result = ext_iterator_directory_init(&it, &dir, 0);
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( ".", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "..", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "lost+found", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "fmove.txt", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "fmovelongname.txt", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "foobarlongfolder", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "fremove.txt", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "fremovelongname.txt", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "hello", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "lorem.txt", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "move", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "movefail", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "movelongname", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "remove", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "removefail", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "removelongname", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_DIR );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_EQ( 0, strncmp( "world.txt", it.entry->name, it.entry->name_len ) );
  EXPECT_EQ( it.entry->file_type, EXT_DIRECTORY_EXT2_FT_REG_FILE );

  // get next entry
  result = ext_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_FALSE( it.entry );

  // finish
  result = ext_iterator_directory_fini( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_FALSE( it.entry );

  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_util_dir_get_non_existing_by_name ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = ext_directory_open( &dir, "/ext2/hello" );
  EXPECT_EQ( result, EOK );

  result = ext_directory_entry_by_name( &dir, "foo" );
  EXPECT_EQ( result, ENOENT );
  EXPECT_FALSE( dir.entry );

  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_new_dir_folder ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/ext2/" );
  EXPECT_TRUE( mp );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // create new folder
  int result = ext_directory_make( "/ext2/hello/foobar" );
  EXPECT_EQ( result, EOK );
  // load root dir
  result = ext_directory_open( &dir, "/ext2/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find created folder
  result = ext_directory_entry_by_name( &dir, "foobar" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // create new folder
  result = ext_directory_make( "/ext2/hello/foobar" );
  EXPECT_EQ( result, EEXIST );
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_new_dir_folder_long ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/ext2/" );
  EXPECT_TRUE( mp );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // create new folder
  int result = ext_directory_make( "/ext2/hello/thisisalongname" );
  EXPECT_EQ( result, EOK );
  // load root dir
  result = ext_directory_open( &dir, "/ext2/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find created folder
  result = ext_directory_entry_by_name( &dir, "thisisalongname" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // create new folder
  result = ext_directory_make( "/ext2/hello/thisisalongname" );
  EXPECT_EQ( result, EEXIST );
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_remove_dir_ro_fail ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  // try to remove directory
  int result = ext_directory_remove( "/ext2/hello/folder/remove/" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = ext_directory_open( &dir, "/ext2/hello/folder/remove/" );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( dir.entry_size, 2 );
  // close directory again
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_remove_dir_rw_success ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // try to remove directory
  int result = ext_directory_remove( "/ext2/hello/folder/remove/" );
  EXPECT_EQ( result, EOK );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = ext_directory_open( &dir, "/ext2/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get entry by name
  result = ext_directory_entry_by_name( &dir, "remove" );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_remove_dir_rw_notempty ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // try to remove directory
  int result = ext_directory_remove( "/ext2/hello/folder/removefail/" );
  EXPECT_EQ( result, ENOTEMPTY );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = ext_directory_open( &dir, "/ext2/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get entry by name
  result = ext_directory_entry_by_name( &dir, "removefail" );
  EXPECT_EQ( result, EOK );
  // close directory again
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_remove_dir_rw_longname ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // try to remove directory
  int result = ext_directory_remove( "/ext2/hello/folder/removelongname/" );
  EXPECT_EQ( result, EOK );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = ext_directory_open( &dir, "/ext2/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get entry by name
  result = ext_directory_entry_by_name( &dir, "removelongname" );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_ro_fail ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  // try to remove directory
  int result = ext_directory_move( "/ext2/hello/folder/move", "/ext2/hello/folder/move2" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = ext_directory_open( &dir, "/ext2/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = ext_directory_entry_by_name( &dir, "move" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_rw_notempty ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // try to remove directory
  int result = ext_directory_move( "/ext2/hello/folder/movefail", "/ext2/hello/folder/move2" );
  EXPECT_EQ( result, ENOTEMPTY );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = ext_directory_open( &dir, "/ext2/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = ext_directory_entry_by_name( &dir, "movefail" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = ext_directory_entry_by_name( &dir, "move2" );
  EXPECT_EQ( result, ENOENT );
  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_rw_exist ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // try to remove directory
  int result = ext_directory_move( "/ext2/hello/folder/move", "/ext2/hello/folder/movefail" );
  EXPECT_EQ( result, EEXIST );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = ext_directory_open( &dir, "/ext2/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = ext_directory_entry_by_name( &dir, "movefail" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_source_not_exist_fail ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // try to remove directory
  int result = ext_directory_move( "/ext2/hello/folder/moveasdf", "/ext2/hello/folder/moveasdf2" );
  EXPECT_EQ( result, ENOENT );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = ext_directory_open( &dir, "/ext2/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = ext_directory_entry_by_name( &dir, "movefail" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_rw_short_name_success ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // try to remove directory
  int result = ext_directory_move( "/ext2/hello/folder/move", "/ext2/hello/folder/move2" );
  EXPECT_EQ( result, EOK );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = ext_directory_open( &dir, "/ext2/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = ext_directory_entry_by_name( &dir, "MOVE" );
  EXPECT_EQ( result, ENOENT );
  // get by name
  result = ext_directory_entry_by_name( &dir, "move2" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // try to revert directory move
  result = ext_directory_move( "/ext2/hello/folder/move2", "/ext2/hello/folder/MOVE" );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = ext_directory_open( &dir, "/ext2/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = ext_directory_entry_by_name( &dir, "MOVE" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = ext_directory_entry_by_name( &dir, "move2" );
  EXPECT_EQ( result, ENOENT );
  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_move_dir_rw_long_name_success ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  // try to remove directory
  int result = ext_directory_move( "/ext2/hello/folder/movelongname", "/ext2/hello/folder/movelongname2" );
  EXPECT_EQ( result, EOK );
  // directory variable
  ext_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = ext_directory_open( &dir, "/ext2/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = ext_directory_entry_by_name( &dir, "movelongname" );
  EXPECT_EQ( result, ENOENT );
  // get by name
  result = ext_directory_entry_by_name( &dir, "movelongname2" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // try to revert directory move
  result = ext_directory_move( "/ext2/hello/folder/movelongname2", "/ext2/hello/folder/movelongname" );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = ext_directory_open( &dir, "/ext2/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = ext_directory_entry_by_name( &dir, "movelongname" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = ext_directory_entry_by_name( &dir, "movelongname2" );
  EXPECT_EQ( result, ENOENT );
  // close directory
  result = ext_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}
