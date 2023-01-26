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
#include <fat/cluster.h>
#include <fat/file.h>
#include "../_helper.hh"
#include "gtest/gtest.h"

TEST( fat16, file_open_non_existant_file ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open(
    &file,
    "/fat16/foobarlongfolder/foo/bar/holla.txt",
    "r"
  );
  EXPECT_EQ( result, ENOENT );
  EXPECT_EQ( file.cluster, 0 );
  EXPECT_FALSE( file.mp );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_open2_non_existant_file ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open2(
    &file,
    "/fat16/foobarlongfolder/foo/bar/holla.txt",
    O_RDONLY
  );
  EXPECT_EQ( result, ENOENT );
  EXPECT_EQ( file.cluster, 0 );
  EXPECT_FALSE( file.mp );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_open_existant_file ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
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
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_open2_existant_file ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open2(
    &file,
    "/fat16/foobarlongfolder/foo/bar/hello.txt",
    O_RDONLY
  );
  EXPECT_EQ( result, EOK );
  EXPECT_NE( file.cluster, 0 );
  EXPECT_NE( file.fsize, 0 );
  EXPECT_TRUE( file.mp );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_open_create_file_rofs ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to create file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/asdf.txt", O_RDWR | O_CREAT );
  EXPECT_EQ( result, EROFS );
  // open /hello and check for asdf
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = fat_directory_open( &dir, "/fat16/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find asdf
  result = fat_directory_entry_by_name( &dir, "asdf.txt" );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_open_create_file_rwfs ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to create file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/asdf.txt", O_RDWR | O_CREAT );
  EXPECT_EQ( result, EOK );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open /hello and check for asdf
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = fat_directory_open( &dir, "/fat16/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find asdf
  result = fat_directory_entry_by_name( &dir, "asdf.txt" );
  EXPECT_EQ( result, EOK );
  EXPECT_STREQ( dir.data->name, "asdf.txt" );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_open_truncate_file ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to create file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2(
    &file,
    "/fat16/hello/file/open/truncate.txt",
    O_RDWR
  );
  EXPECT_EQ( result, EOK );
  EXPECT_NE( file.fsize, 0 );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open with truncate
  result = fat_file_open2(
    &file,
    "/fat16/hello/file/open/truncate.txt",
    O_RDWR | O_TRUNC
  );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, 0 );
  // close file again
  result = fat_file_close( &file );
  // umount
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_open_create_file_short_short_ext ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to create file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/asd1.txt", O_RDWR | O_CREAT );
  EXPECT_EQ( result, EOK );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open /hello and check for asdf
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = fat_directory_open( &dir, "/fat16/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find asdf
  result = fat_directory_entry_by_name( &dir, "asd1.txt" );
  EXPECT_EQ( result, EOK );
  EXPECT_STREQ( dir.data->name, "asd1.txt" );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_open_create_file_short_long_ext ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to create file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/asd2.jpeg", O_RDWR | O_CREAT );
  EXPECT_EQ( result, EOK );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open /hello and check for asdf
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = fat_directory_open( &dir, "/fat16/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find asdf
  result = fat_directory_entry_by_name( &dir, "asd2.jpeg" );
  EXPECT_EQ( result, EOK );
  EXPECT_STREQ( dir.data->name, "asd2.jpeg" );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_open_create_file_long_short_ext ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to create file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/asdftolongname.txt", O_RDWR | O_CREAT );
  EXPECT_EQ( result, EOK );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open /hello and check for asdf
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = fat_directory_open( &dir, "/fat16/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find asdf
  result = fat_directory_entry_by_name( &dir, "asdftolongname.txt" );
  EXPECT_EQ( result, EOK );
  EXPECT_STREQ( dir.data->name, "asdftolongname.txt" );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_open_create_file_long_long_ext ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to create file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/asdftolongname2.jpeg", O_RDWR | O_CREAT );
  EXPECT_EQ( result, EOK );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open /hello and check for asdf
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  result = fat_directory_open( &dir, "/fat16/hello/" );
  EXPECT_EQ( result, EOK );
  // try to find asdf
  result = fat_directory_entry_by_name( &dir, "asdftolongname2.jpeg" );
  EXPECT_EQ( result, EOK );
  EXPECT_STREQ( dir.data->name, "asdftolongname2.jpeg" );
  // close directory again
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // umount
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_read_normal1 ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
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
  char* buffer = ( char* )malloc( file.fsize + 1 );
  EXPECT_TRUE( buffer );
  // read from file
  uint64_t read_count = 0;
  result = fat_file_read( &file, buffer, file.fsize, &read_count );
  buffer[ file.fsize ] = '\0';
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( read_count, file.fsize );
  EXPECT_STREQ( buffer, "world\n" );
  // free again
  free( buffer );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_read_normal2 ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open( &file, "/fat16/hello/world.txt", "r" );
  EXPECT_EQ( result, EOK );
  EXPECT_NE( file.cluster, 0 );
  EXPECT_NE( file.fsize, 0 );
  EXPECT_TRUE( file.mp );
  // allocate buffer for content
  char* buffer = ( char* )malloc( file.fsize + 1 );
  EXPECT_TRUE( buffer );
  // read from file
  uint64_t read_count = 0;
  result = fat_file_read( &file, buffer, file.fsize, &read_count );
  buffer[ file.fsize ] = '\0';
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( read_count, file.fsize );
  EXPECT_STREQ( buffer, "hello world\n" );
  // free again
  free( buffer );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_read_rootdir ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open( &file, "/fat16/world.txt", "r" );
  EXPECT_EQ( result, EOK );
  EXPECT_NE( file.cluster, 0 );
  EXPECT_NE( file.fsize, 0 );
  EXPECT_TRUE( file.mp );
  // allocate buffer for content
  char* buffer = ( char* )malloc( file.fsize + 1 );
  EXPECT_TRUE( buffer );
  // read from file
  uint64_t read_count = 0;
  result = fat_file_read( &file, buffer, file.fsize, &read_count );
  buffer[ file.fsize ] = '\0';
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( read_count, file.fsize );
  EXPECT_STREQ( buffer, "hello world\n" );
  // free again
  free( buffer );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_read_write_only ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // file variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // load root dir
  int result = fat_file_open2( &file, "/fat16/world.txt", O_WRONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_NE( file.cluster, 0 );
  EXPECT_NE( file.fsize, 0 );
  EXPECT_TRUE( file.mp );
  // allocate buffer for content
  char* buffer = ( char* )malloc( file.fsize + 1 );
  EXPECT_TRUE( buffer );
  memset( buffer, 0, file.fsize + 1 );
  // read from file
  uint64_t read_count = 0;
  result = fat_file_read( &file, buffer, file.fsize, &read_count );
  EXPECT_EQ( result, EPERM );
  EXPECT_EQ( read_count, 0 );
  EXPECT_STRNE( buffer, "hello world\n" );
  // free again
  free( buffer );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

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

TEST( fat16, file_ftruncate_ro ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/trunc/extend.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  // save old size
  uint64_t old_size = file.fsize;
  // call for truncate
  result = fat_file_truncate( &file, old_size * 2 );
  EXPECT_EQ( result, EPERM );
  EXPECT_EQ( file.fsize, old_size );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat16/hello/trunc/extend.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, old_size );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_ftruncate_extend_cluster ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/trunc/extend.txt", O_RDWR );
  EXPECT_EQ( result, EOK );
  // save old size
  fat_fs_t* fs = ( fat_fs_t* )file.mp->fs;
  uint64_t cluster_size = fs->superblock.bytes_per_sector
      * fs->superblock.sectors_per_cluster;
  uint64_t old_count = file.fsize / cluster_size;
  if ( file.fsize % cluster_size ) {
    old_count++;
  }
  uint64_t truncate_size = ( old_count + 1 ) * cluster_size - 1;
  uint64_t new_count = truncate_size / cluster_size;
  if ( truncate_size % cluster_size ) {
    new_count++;
  }
  // call for truncate
  result = fat_file_truncate( &file, truncate_size );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, truncate_size );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat16/hello/trunc/extend.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, truncate_size );
  // get last cluster
  uint64_t val;
  result = fat_cluster_get_by_num(
    fs,
    file.cluster,
    file.fsize / ( fs->superblock.sectors_per_cluster
      * fs->superblock.bytes_per_sector ),
    &val
  );

  // get custer chain end value by type
  uint64_t end_value;
  result = fat_cluster_get_chain_end_value( fs, &end_value );
  EXPECT_EQ( result, EOK );
  uint64_t cluster_count = 1;
  uint64_t current_cluster = file.cluster;
  // check cluster chain
  while ( true ) {
    // read cluster
    uint64_t next_cluster;
    result = fat_cluster_next( fs, current_cluster, &next_cluster );
    EXPECT_EQ( result, EOK );
    // handle end reached
    if ( next_cluster >= end_value ) {
      break;
    }
    current_cluster = next_cluster;
    cluster_count++;
  }
  EXPECT_EQ( cluster_count, new_count );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_ftruncate_extend_cluster_times_n ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  for ( uint64_t index = 0; index < 10; index++ ) {
    // open file
    fat_file_t file;
    memset( &file, 0, sizeof( file ) );
    int result = fat_file_open2( &file, "/fat16/hello/trunc/extend.txt", O_RDWR );
    EXPECT_EQ( result, EOK );
    // save old size
    fat_fs_t* fs = ( fat_fs_t* )file.mp->fs;
    uint64_t cluster_size = fs->superblock.bytes_per_sector
        * fs->superblock.sectors_per_cluster;
    uint64_t old_count = file.fsize / cluster_size;
    if ( file.fsize % cluster_size ) {
      old_count++;
    }
    uint64_t truncate_size = ( old_count + 1 ) * cluster_size - 1;
    uint64_t new_count = truncate_size / cluster_size;
    if ( truncate_size % cluster_size ) {
      new_count++;
    }
    // call for truncate
    result = fat_file_truncate( &file, truncate_size );
    EXPECT_EQ( result, EOK );
    EXPECT_EQ( file.fsize, truncate_size );
    // close file
    result = fat_file_close( &file );
    EXPECT_EQ( result, EOK );
    // open file again
    result = fat_file_open2( &file, "/fat16/hello/trunc/extend.txt", O_RDONLY );
    EXPECT_EQ( result, EOK );
    EXPECT_EQ( file.fsize, truncate_size );
    // get last cluster
    uint64_t val;
    result = fat_cluster_get_by_num(
      fs,
      file.cluster,
      file.fsize / ( fs->superblock.sectors_per_cluster
        * fs->superblock.bytes_per_sector ),
      &val
    );

    // get custer chain end value by type
    uint64_t end_value;
    result = fat_cluster_get_chain_end_value( fs, &end_value );
    EXPECT_EQ( result, EOK );
    uint64_t cluster_count = 1;
    uint64_t current_cluster = file.cluster;
    // check cluster chain
    while ( true ) {
      // read cluster
      uint64_t next_cluster;
      result = fat_cluster_next( fs, current_cluster, &next_cluster );
      EXPECT_EQ( result, EOK );
      // handle end reached
      if ( next_cluster >= end_value ) {
        break;
      }
      current_cluster = next_cluster;
      cluster_count++;
    }
    EXPECT_EQ( cluster_count, new_count );
    // close file again
    result = fat_file_close( &file );
    EXPECT_EQ( result, EOK );
  }
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_ftruncate_shrink_cluster ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/trunc/shrink.txt", O_RDWR );
  EXPECT_EQ( result, EOK );
  // save old size
  fat_fs_t* fs = ( fat_fs_t* )file.mp->fs;
  uint64_t cluster_size = fs->superblock.bytes_per_sector
      * fs->superblock.sectors_per_cluster;
  uint64_t old_count = file.fsize / cluster_size + 1;
  uint64_t truncate_size = ( old_count + 1 ) * cluster_size - 1;
  uint64_t new_count = truncate_size / cluster_size;
  if ( truncate_size % cluster_size ) {
    new_count++;
  }
  // call for truncate
  result = fat_file_truncate( &file, truncate_size );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, truncate_size );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat16/hello/trunc/shrink.txt", O_RDWR );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, truncate_size );
  // call for truncate
  result = fat_file_truncate( &file, 50 );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, 50 );
  // get custer chain end value by type
  uint64_t end_value;
  result = fat_cluster_get_chain_end_value( fs, &end_value );
  EXPECT_EQ( result, EOK );
  uint64_t cluster_count = 1;
  uint64_t current_cluster = file.cluster;
  // check cluster chain
  while ( true ) {
    // read cluster
    uint64_t next_cluster;
    result = fat_cluster_next( fs, current_cluster, &next_cluster );
    EXPECT_EQ( result, EOK );
    // handle end reached
    if ( next_cluster >= end_value ) {
      break;
    }
    current_cluster = next_cluster;
    cluster_count++;
  }
  EXPECT_EQ( cluster_count, old_count );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_ftruncate_extend_size_only ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/trunc/extend2.txt", O_RDWR );
  EXPECT_EQ( result, EOK );
  // save old size
  uint64_t old_size = file.fsize;
  // call for truncate
  result = fat_file_truncate( &file, old_size * 2 );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, old_size * 2 );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat16/hello/trunc/extend2.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, old_size * 2 );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_ftruncate_shrink_size_only ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/trunc/shrink2.txt", O_RDWR );
  EXPECT_EQ( result, EOK );
  // save old size
  uint64_t old_size = file.fsize;
  // call for truncate
  result = fat_file_truncate( &file, old_size / 2 );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, old_size / 2 );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat16/hello/trunc/shrink2.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, old_size / 2 );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_ftruncate_free_cluster_size ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat16/hello/trunc/shrink2.txt", O_RDWR );
  EXPECT_EQ( result, EOK );
  // save old size
  uint64_t old_size = file.fsize;
  // call for truncate
  result = fat_file_truncate( &file, 0 );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, 0 );
  EXPECT_EQ( file.cluster, 0 );
  EXPECT_EQ( file.dentry->first_cluster_lower, 0 );
  EXPECT_EQ( file.dentry->first_cluster_upper, 0 );
  // close file
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat16/hello/trunc/shrink2.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, 0 );
  EXPECT_EQ( file.cluster, 0 );
  EXPECT_EQ( file.dentry->first_cluster_lower, 0 );
  EXPECT_EQ( file.dentry->first_cluster_upper, 0 );
  // close file again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_remove_rootdir_ro_fail ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_remove( "/fat16/fremove.txt" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat16/fremove.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, strlen("hello world\n") );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_remove_rootdir_rw_success ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_remove( "/fat16/fremove.txt" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat16/fremove.txt", O_RDONLY );
  EXPECT_EQ( result, ENOENT );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_remove_rootdir_rw_longname ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_remove( "/fat16/fremovelongname.txt" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat16/fremovelongname.txt", O_RDONLY );
  EXPECT_EQ( result, ENOENT );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_remove_dir_ro_fail ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_remove( "/fat16/hello/file/remove.txt" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat16/hello/file/remove.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, strlen("hello world\n") );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_remove_dir_rw_success ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_remove( "/fat16/hello/file/remove.txt" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat16/hello/file/remove.txt", O_RDONLY );
  EXPECT_EQ( result, ENOENT );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_remove_dir_rw_longname ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_remove( "/fat16/hello/file/removelongname.txt" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat16/hello/file/removelongname.txt", O_RDONLY );
  EXPECT_EQ( result, ENOENT );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_move_rootdir_ro_fail ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_move( "/fat16/fmove.txt", "/fat16/fmove2.txt" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat16/fmove.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, strlen("hello world\n") );
  // open base directory
  result = fat_file_open2( &file, "/fat16/fmove2.txt", O_RDONLY );
  EXPECT_EQ( result, ENOENT );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_move_rootdir_target_exist_fail ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_move( "/fat16/fmove.txt", "/fat16/world.txt" );
  EXPECT_EQ( result, EEXIST );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat16/fmove.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, strlen("hello world\n") );
  // close again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = fat_file_open2( &file, "/fat16/world.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_move_rootdir_source_not_exist_fail ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_move( "/fat16/fmoveinvalid.txt", "/fat16/world2.txt" );
  EXPECT_EQ( result, ENOENT );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat16/world.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_move_rootdir_rw_short_name_success ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_move( "/fat16/fmove.txt", "/fat16/fmove2.txt" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat16/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmove.txt" );
  EXPECT_EQ( result, ENOENT );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmove2.txt" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // try to revert directory move
  result = fat_file_move( "/fat16/fmove2.txt", "/fat16/fmove.txt" );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = fat_directory_open( &dir, "/fat16/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmove.txt" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmove2.txt" );
  EXPECT_EQ( result, ENOENT );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_move_rootdir_rw_long_name_success ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_move( "/fat16/fmovelongname.txt", "/fat16/fmovelongname2.txt" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat16/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmovelongname.txt" );
  EXPECT_EQ( result, ENOENT );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmovelongname2.txt" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // try to revert directory move
  result = fat_file_move( "/fat16/fmovelongname2.txt", "/fat16/fmovelongname.txt" );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = fat_directory_open( &dir, "/fat16/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmovelongname.txt" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmovelongname2.txt" );
  EXPECT_EQ( result, ENOENT );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_move_dir_ro_fail ) {
  helper_mount_test_image( true, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_move( "/fat16/hello/file/fmove.txt", "/fat16/hello/file/fmove2.txt" );
  EXPECT_EQ( result, EROFS );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat16/hello/file/fmove.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, strlen("hello world\n") );
  // close again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = fat_file_open2( &file, "/fat16/hello/file/fmove2.txt", O_RDONLY );
  EXPECT_EQ( result, ENOENT );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_move_dir_target_exist_fail ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_move( "/fat16/hello/file/fmove.txt", "/fat16/hello/world.txt" );
  EXPECT_EQ( result, EEXIST );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat16/hello/file/fmove.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  EXPECT_EQ( file.fsize, strlen("hello world\n") );
  // close again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = fat_file_open2( &file, "/fat16/hello/world.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  // close again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_move_dir_source_not_exist_fail ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_move( "/fat16/hello/file/fmoveinvalid.txt", "/fat16/world2.txt" );
  EXPECT_EQ( result, ENOENT );
  // directory variable
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  // open base directory
  result = fat_file_open2( &file, "/fat16/world.txt", O_RDONLY );
  EXPECT_EQ( result, EOK );
  // close directory again
  result = fat_file_close( &file );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_move_dir_rw_short_name_success ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_move( "/fat16/hello/file/fmove.txt", "/fat16/hello/file/fmove2.txt" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat16/hello/file/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmove.txt" );
  EXPECT_EQ( result, ENOENT );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmove2.txt" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // try to revert directory move
  result = fat_file_move( "/fat16/hello/file/fmove2.txt", "/fat16/hello/file/fmove.txt" );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = fat_directory_open( &dir, "/fat16/hello/file/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmove.txt" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmove2.txt" );
  EXPECT_EQ( result, ENOENT );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}

TEST( fat16, file_move_dir_rw_long_name_success ) {
  helper_mount_test_image( false, "fat16.img", "fat16", "/fat16/", FAT_FAT16 );
  // try to remove directory
  int result = fat_file_move( "/fat16/hello/file/fmovelongname.txt", "/fat16/hello/file/fmovelongname2.txt" );
  EXPECT_EQ( result, EOK );
  // directory variable
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  // open base directory
  result = fat_directory_open( &dir, "/fat16/hello/file/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmovelongname.txt" );
  EXPECT_EQ( result, ENOENT );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmovelongname2.txt" );
  EXPECT_EQ( result, EOK );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // try to revert directory move
  result = fat_file_move( "/fat16/hello/file/fmovelongname2.txt", "/fat16/hello/file/fmovelongname.txt" );
  EXPECT_EQ( result, EOK );
  // open base directory
  result = fat_directory_open( &dir, "/fat16/" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmovelongname.txt" );
  EXPECT_EQ( result, EOK );
  // get by name
  result = fat_directory_entry_by_name( &dir, "fmovelongname2.txt" );
  EXPECT_EQ( result, ENOENT );
  // close directory
  result = fat_directory_close( &dir );
  EXPECT_EQ( result, EOK );
  // unmount test image
  helper_unmount_test_image( "fat16", "/fat16/" );
}
