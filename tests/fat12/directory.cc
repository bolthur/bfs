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

TEST( fat12, directory_open_sub_directory ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_directory_open( &dir, "/fat12/foobarlongfolder/foo/bar" );
  EXPECT_EQ( result, EOK );

  fat_iterator_directory_t it;
  memset( &it, 0, sizeof( it ) );
  result = fat_iterator_directory_init(&it, &dir, 0);
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "HELLO.TXT", it.data->name );

  // finish
  result = fat_iterator_directory_fini( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_FALSE( it.entry );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, root_directory_read_dir_utils ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_rootdir_open( mp, &dir );
  EXPECT_EQ( result, EOK );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "movelongname", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "fmovelongname.txt", dir.data->name );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "REMOVE", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "HELLO", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "fremovelongname.txt", dir.data->name );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "MOVEFAIL", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "MOVE", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "WORLD.TXT", dir.data->name );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "removefail", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "LOREM.TXT", dir.data->name );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "FMOVE.TXT", dir.data->name );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "removelongname", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "FREMOVE.TXT", dir.data->name );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "foobarlongfolder", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, root_directory_read_dir_utils_rewind ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_rootdir_open( mp, &dir );
  EXPECT_EQ( result, EOK );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "movelongname", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "fmovelongname.txt", dir.data->name );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "REMOVE", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // rewind
  result = fat_directory_rewind( &dir );
  EXPECT_EQ( result, EOK );

  // get next entry
  result = fat_directory_next_entry( &dir );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "movelongname", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_get_by_name ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // open file
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open directory
  int result = fat_directory_open( &dir, "/fat12/hello/" );
  EXPECT_EQ( result, EOK );
  // get entry
  result = fat_directory_entry_by_name( &dir, "truncate.txt" );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "TRUNCATE.TXT", dir.data->name );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount again
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_iterator_root_dir_read ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_rootdir_open( mp, &dir );
  EXPECT_EQ( result, EOK );

  fat_iterator_directory_t it;
  memset( &it, 0, sizeof( it ) );
  result = fat_iterator_directory_init(&it, &dir, 0);
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "movelongname", it.data->name );
  EXPECT_EQ( it.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "fmovelongname.txt", it.data->name );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "REMOVE", it.data->name );
  EXPECT_EQ( it.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "HELLO", it.data->name );
  EXPECT_EQ( it.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "fremovelongname.txt", it.data->name );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "MOVEFAIL", it.data->name );
  EXPECT_EQ( it.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "MOVE", it.data->name );
  EXPECT_EQ( it.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "WORLD.TXT", it.data->name );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "removefail", it.data->name );
  EXPECT_EQ( it.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "LOREM.TXT", it.data->name );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "FMOVE.TXT", it.data->name );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "removelongname", it.data->name );
  EXPECT_EQ( it.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "FREMOVE.TXT", it.data->name );

  // get next entry
  result = fat_iterator_directory_next( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( it.entry );
  EXPECT_STREQ( "foobarlongfolder", it.data->name );

  // finish
  result = fat_iterator_directory_fini( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_FALSE( it.entry );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_util_root_dir_get_existing_by_name ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_rootdir_open( mp, &dir );
  EXPECT_EQ( result, EOK );

  result = fat_directory_entry_by_name( &dir, "foobarlongfolder" );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "foobarlongfolder", dir.data->name );
  EXPECT_EQ( dir.entry->attributes, FAT_DIRECTORY_FILE_ATTRIBUTE_DIRECTORY );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_util_dir_get_existing_by_name ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_directory_open( &dir, "/fat12/hello" );
  EXPECT_EQ( result, EOK );

  result = fat_directory_entry_by_name( &dir, "world.txt" );
  EXPECT_EQ( result, EOK );
  EXPECT_TRUE( dir.entry );
  EXPECT_TRUE( dir.data );
  EXPECT_STREQ( "WORLD.TXT", dir.data->name );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_util_root_dir_get_non_existing_by_name ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_rootdir_open( mp, &dir );
  EXPECT_EQ( result, EOK );

  result = fat_directory_entry_by_name( &dir, "nonexistant" );
  EXPECT_EQ( result, ENOENT );
  EXPECT_FALSE( dir.entry );
  EXPECT_FALSE( dir.data );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_util_dir_get_non_existing_by_name ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_directory_open( &dir, "/fat12/hello" );
  EXPECT_EQ( result, EOK );

  result = fat_directory_entry_by_name( &dir, "foo" );
  EXPECT_EQ( result, ENOENT );
  EXPECT_FALSE( dir.entry );
  EXPECT_FALSE( dir.data );

  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );

  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_new_root_dir_folder ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // create new folder
  int result = fat_directory_make( "/fat12/wup/" );
  EXPECT_EQ( result, EOK );
  // load root dir
  result = fat_rootdir_open( mp, &dir );
  EXPECT_EQ( result, EOK );
  // try to find created folder
  result = fat_directory_entry_by_name( &dir, "wup" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // create new folder
  result = fat_directory_make( "/fat12/wup/" );
  EXPECT_EQ( result, EEXIST );
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_new_dir_folder ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // create new folder
  int result = fat_directory_make( "/fat12/hello/foobar" );
  EXPECT_EQ( result, EOK );
  // load root dir
  result = fat_directory_open( &dir, "/fat12/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find created folder
  result = fat_directory_entry_by_name( &dir, "foobar" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // create new folder
  result = fat_directory_make( "/fat12/hello/foobar" );
  EXPECT_EQ( result, EEXIST );
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_new_root_dir_folder_long ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // create new folder
  int result = fat_directory_make( "/fat12/thisisalongname/" );
  EXPECT_EQ( result, EOK );
  // load root dir
  result = fat_rootdir_open( mp, &dir );
  EXPECT_EQ( result, EOK );
  // try to find created folder
  result = fat_directory_entry_by_name( &dir, "thisisalongname" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // create new folder
  result = fat_directory_make( "/fat12/thisisalongname/" );
  EXPECT_EQ( result, EEXIST );
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_new_dir_folder_long ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat12/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // create new folder
  int result = fat_directory_make( "/fat12/hello/thisisalongname" );
  EXPECT_EQ( result, EOK );
  // load root dir
  result = fat_directory_open( &dir, "/fat12/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find created folder
  result = fat_directory_entry_by_name( &dir, "thisisalongname" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // create new folder
  result = fat_directory_make( "/fat12/hello/thisisalongname" );
  EXPECT_EQ( result, EEXIST );
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_rootdir_ro_fail ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/remove/" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/remove/" );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( dir.entry_size, 0 );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_rootdir_rw_success ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/remove/" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get entry by name
  result = fat_directory_entry_by_name( &dir, "remove" );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_rootdir_rw_notempty ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/removefail/" );
  EXPECT_EQ( result, ENOTEMPTY );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get entry by name
  result = fat_directory_entry_by_name( &dir, "removefail" );
  EXPECT_EQ( result, EOK );
  EXPECT_STREQ( dir.data->name, "removefail" );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_rootdir_rw_longname ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/removelongname/" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get entry by name
  result = fat_directory_entry_by_name( &dir, "removelongname" );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_dir_ro_fail ) {
  helper_mount_test_image( true, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/hello/folder/remove/" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/hello/folder/remove/" );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( dir.entry_size, 0 );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_dir_rw_success ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/hello/folder/remove/" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get entry by name
  result = fat_directory_entry_by_name( &dir, "remove" );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_dir_rw_notempty ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/hello/folder/removefail/" );
  EXPECT_EQ( result, ENOTEMPTY );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get entry by name
  result = fat_directory_entry_by_name( &dir, "removefail" );
  EXPECT_EQ( result, EOK );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

TEST( fat12, directory_remove_dir_rw_longname ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_remove( "/fat12/hello/folder/removelongname/" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get entry by name
  result = fat_directory_entry_by_name( &dir, "removelongname" );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}

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

TEST( fat12, directory_move_rootdir_rw_short_name_success ) {
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

TEST( fat12, directory_move_rootdir_rw_long_name_success ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_move( "/fat12/movelongname", "/fat12/movelongname2" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movelongname" );
  EXPECT_EQ( result, ENOENT );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movelongname2" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // try to revert directory move
  result = fat_directory_move( "/fat12/movelongname2", "/fat12/movelongname" );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movelongname" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movelongname2" );
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

TEST( fat12, directory_move_dir_rw_short_name_success ) {
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

TEST( fat12, directory_move_dir_rw_long_name_success ) {
  helper_mount_test_image( false, "fat12.img", "fat12", "/fat12/", FAT_FAT12 );
  // try to remove directory
  int result = fat_directory_move( "/fat12/hello/folder/movelongname", "/fat12/hello/folder/movelongname2" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movelongname" );
  EXPECT_EQ( result, ENOENT );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movelongname2" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // try to revert directory move
  result = fat_directory_move( "/fat12/hello/folder/movelongname2", "/fat12/hello/folder/movelongname" );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = fat_directory_open( &dir, "/fat12/hello/folder/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movelongname" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "movelongname2" );
  EXPECT_EQ( result, ENOENT );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat12", "/fat12/" );
}
