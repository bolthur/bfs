/**
 * Copyright (C) 2022 bolthur project.
 *
 * This file is part of bolthur/bfs.
 *
 * bolthur/bfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bolthur/bfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bolthur/bfs.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include "gtest/gtest.h"

/**
 * @brief Helper to mount test image
 */
static void mount_test_image( void ) {
  // get block device
  common_blockdev_t* bdev = common_blockdev_get();
  EXPECT_TRUE( bdev );
  // set blockdev filename
  common_blockdev_set_fname( "fat32.img" );
  /// FIXME: SHOULD BE DONE WITHIN BLOCKDEV OPEN (?)
  bdev->part_offset = bdev->bdif->block_size;
  // register block device
  int result = common_blockdev_register_device( bdev, "fat32" );
  EXPECT_EQ( result, EOK );
  // mount device
  result = fat_mountpoint_mount( "fat32", "/fat32/", true );
  EXPECT_EQ( result, EOK );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 1 );
  // get fs
  fat_fs_t* fs = static_cast< fat_fs_t* >( bdev->fs );
  // assert fs and fs type
  EXPECT_TRUE( fs );
  EXPECT_TRUE( fs->read_only );
  EXPECT_EQ( FAT_FAT32, fs->type );
}

/**
 * @brief Helper to unmount test image
 */
static void unmount_test_image( void ) {
  // get block device
  common_blockdev_t* bdev = common_blockdev_get();
  EXPECT_TRUE( bdev );
  // unmount
  int result = fat_mountpoint_umount( "/fat32/" );
  EXPECT_EQ( result, EOK );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 0 );
  // unregister device
  result = common_blockdev_unregister_device( "fat32" );
  EXPECT_EQ( result, EOK );
}

// Demonstrate some basic assertions.
TEST( FAT32, MountNullBlockDevice ) {
  // get block device
  common_blockdev_t* bdev = common_blockdev_get();
  EXPECT_TRUE( bdev );
  // set blockdev filename
  common_blockdev_set_fname( NULL );
  /// FIXME: SHOULD BE DONE WITHIN BLOCKDEV OPEN (?)
  bdev->part_offset = bdev->bdif->block_size;
  // register block device
  int result = common_blockdev_register_device( bdev, "fat32" );
  EXPECT_EQ( result, EOK );
  // mount device
  result = fat_mountpoint_mount( "fat32", "/fat32/", true );
  EXPECT_EQ( result, ENODATA );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 0 );
  // get fs
  fat_fs_t* fs = static_cast< fat_fs_t* >( bdev->fs );
  // assert fs and fs type
  EXPECT_FALSE( fs );
  // unmount
  result = fat_mountpoint_umount( "/fat32/" );
  EXPECT_EQ( result, ENODEV );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 0 );
  // unregister device
  result = common_blockdev_unregister_device( "fat32" );
  EXPECT_EQ( result, EOK );
}

// Demonstrate some basic assertions.
TEST( FAT32, MountInvalidBlockDevice ) {
  // get block device
  common_blockdev_t* bdev = common_blockdev_get();
  EXPECT_TRUE( bdev );
  // set blockdev filename
  common_blockdev_set_fname( "fat.img" );
  /// FIXME: SHOULD BE DONE WITHIN BLOCKDEV OPEN (?)
  bdev->part_offset = bdev->bdif->block_size;
  // register block device
  int result = common_blockdev_register_device( bdev, "fat32" );
  EXPECT_EQ( result, EOK );
  // mount device
  result = fat_mountpoint_mount( "fat32", "/fat32/", true );
  EXPECT_EQ( result, EIO );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 0 );
  // get fs
  fat_fs_t* fs = static_cast< fat_fs_t* >( bdev->fs );
  // assert fs and fs type
  EXPECT_FALSE( fs );
  // unmount
  result = fat_mountpoint_umount( "/fat32/" );
  EXPECT_EQ( result, ENODEV );
  // assert count
  EXPECT_EQ( bdev->bdif->reference_counter, 0 );
  // unregister device
  result = common_blockdev_unregister_device( "fat32" );
  EXPECT_EQ( result, EOK );
}

// Demonstrate some basic assertions.
TEST( FAT32, NormalMountRo ) {
  mount_test_image();
  unmount_test_image();
}

// Demonstrate some basic assertions.
TEST( FAT32, NormalMountRw ) {
  mount_test_image();
  unmount_test_image();
}

// Demonstrate some basic assertions.
TEST( FAT32, RootDirRead ) {
  mount_test_image();
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
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
  EXPECT_STREQ( "HELLO", it.data->name );
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
  EXPECT_STREQ( "foobarlongfolder", it.data->name );

  // finish
  result = fat_iterator_directory_fini( &it );
  EXPECT_EQ( result, EOK );
  EXPECT_FALSE( it.entry );
  unmount_test_image();
}

// Demonstrate some basic assertions.
TEST( FAT32, OpenSubDirectory ) {
  mount_test_image();
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/fat32/" );
  EXPECT_TRUE( mp );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // load root dir
  int result = fat_directory_open( &dir, "/fat32/foobarlongfolder/foo/bar" );
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
  unmount_test_image();
}

// Demonstrate some basic assertions.
TEST( FAT32, OpenNonExistantFile ) {
  mount_test_image();
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open(
    &file,
    "/fat32/foobarlongfolder/foo/bar/holla.txt",
    "r"
  );
  EXPECT_EQ( result, ENOENT );
  EXPECT_EQ( file.cluster, 0 );
  EXPECT_FALSE( file.mp );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  unmount_test_image();
}

// Demonstrate some basic assertions.
TEST( FAT32, Open2NonExistantFile ) {
  mount_test_image();
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open2(
    &file,
    "/fat32/foobarlongfolder/foo/bar/holla.txt",
    O_RDONLY
  );
  EXPECT_EQ( result, ENOENT );
  EXPECT_EQ( file.cluster, 0 );
  EXPECT_FALSE( file.mp );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  unmount_test_image();
}

// Demonstrate some basic assertions.
TEST( FAT32, OpenExistantFile ) {
  mount_test_image();
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open(
    &file,
    "/fat32/foobarlongfolder/foo/bar/hello.txt",
    "r"
  );
  EXPECT_EQ( result, EOK );
  EXPECT_NE( file.cluster, 0 );
  EXPECT_TRUE( file.mp );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  unmount_test_image();
}

// Demonstrate some basic assertions.
TEST( FAT32, Open2ExistantFile ) {
  mount_test_image();
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open2(
    &file,
    "/fat32/foobarlongfolder/foo/bar/hello.txt",
    O_RDONLY
  );
  EXPECT_EQ( result, EOK );
  EXPECT_NE( file.cluster, 0 );
  EXPECT_TRUE( file.mp );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  unmount_test_image();
}
