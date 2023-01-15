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
#include <fat/cluster.h>
#include <fat/iterator.h>
#include <fat/fs.h>
#include <fat/file.h>
#include <check.h>
#include "../_helper.h"

START_TEST( test_file_ftruncate_ro ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat32/hello/truncate.txt", O_RDONLY );
  ck_assert_int_eq( result, EOK );
  // save old size
  uint64_t old_size = file.fsize;
  // call for truncate
  result = fat_file_truncate( &file, old_size * 2 );
  ck_assert_int_eq( result, EINVAL );
  ck_assert_uint_eq( file.fsize, old_size );
  // close file
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat32/hello/truncate.txt", O_RDONLY );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, old_size );
  // close file again
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

START_TEST( test_file_ftruncate_extend_cluster ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat32/hello/truncate.txt", O_RDWR );
  ck_assert_int_eq( result, EOK );
  // save old size
  fat_fs_t* fs = file.mp->fs;
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
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, truncate_size );
  // close file
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat32/hello/truncate.txt", O_RDONLY );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, truncate_size );
  // get last cluster
  uint64_t val;
  result = fat_cluster_get_by_num(
    file.mp->fs,
    file.cluster,
    file.fsize / ( fs->superblock.sectors_per_cluster
      * fs->superblock.bytes_per_sector ),
    &val
  );

  // get custer chain end value by type
  uint64_t end_value;
  result = fat_cluster_get_chain_end_value( file.mp->fs, &end_value );
  ck_assert_int_eq( result, EOK );
  uint64_t cluster_count = 1;
  uint64_t current_cluster = file.cluster;
  // check cluster chain
  while ( true ) {
    // read cluster
    uint64_t next_cluster;
    result = fat_cluster_next( file.mp->fs, current_cluster, &next_cluster );
    ck_assert_int_eq( result, EOK );
    // handle end reached
    if ( next_cluster >= end_value ) {
      break;
    }
    current_cluster = next_cluster;
    cluster_count++;
  }
  ck_assert_uint_eq( cluster_count, new_count );
  // close file again
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

START_TEST( test_file_ftruncate_shrink_cluster ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat32/hello/truncate.txt", O_RDWR );
  ck_assert_int_eq( result, EOK );
  // save old size
  fat_fs_t* fs = file.mp->fs;
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
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, truncate_size );
  // close file
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat32/hello/truncate.txt", O_RDWR );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, truncate_size );
  // call for truncate
  result = fat_file_truncate( &file, 50 );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, 50 );
  // get custer chain end value by type
  uint64_t end_value;
  result = fat_cluster_get_chain_end_value( file.mp->fs, &end_value );
  ck_assert_int_eq( result, EOK );
  uint64_t cluster_count = 1;
  uint64_t current_cluster = file.cluster;
  // check cluster chain
  while ( true ) {
    // read cluster
    uint64_t next_cluster;
    result = fat_cluster_next( file.mp->fs, current_cluster, &next_cluster );
    ck_assert_int_eq( result, EOK );
    // handle end reached
    if ( next_cluster >= end_value ) {
      break;
    }
    current_cluster = next_cluster;
    cluster_count++;
  }
  ck_assert_uint_eq( cluster_count, new_count );
  // close file again
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

START_TEST( test_file_ftruncate_extend_size_only ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat32/hello/truncate.txt", O_RDWR );
  ck_assert_int_eq( result, EOK );
  // save old size
  uint64_t old_size = file.fsize;
  // call for truncate
  result = fat_file_truncate( &file, old_size * 2 );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, old_size * 2 );
  // close file
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat32/hello/truncate.txt", O_RDONLY );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, old_size * 2 );
  // close file again
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

START_TEST( test_file_ftruncate_shrink_size_only ) {
  helper_mount_test_image( false, "fat32.img", "fat32", "/fat32/", FAT_FAT32 );
  // open file
  fat_file_t file;
  memset( &file, 0, sizeof( file ) );
  int result = fat_file_open2( &file, "/fat32/hello/truncate.txt", O_RDWR );
  ck_assert_int_eq( result, EOK );
  // save old size
  uint64_t old_size = file.fsize;
  // call for truncate
  result = fat_file_truncate( &file, old_size / 2 );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, old_size / 2 );
  // close file
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  // open file again
  result = fat_file_open2( &file, "/fat32/hello/truncate.txt", O_RDONLY );
  ck_assert_int_eq( result, EOK );
  ck_assert_uint_eq( file.fsize, old_size / 2 );
  // close file again
  result = fat_file_close( &file );
  ck_assert_int_eq( result, EOK );
  helper_unmount_test_image( "fat32", "/fat32/" );
}
END_TEST

Suite* fat32_suite_file_ftruncate( void ) {
  Suite* s = suite_create( "fat32_file_ftruncate" );
  TCase* tc_core = tcase_create( "fat32" );
  tcase_add_test( tc_core, test_file_ftruncate_ro );
  tcase_add_test( tc_core, test_file_ftruncate_extend_cluster );
  tcase_add_test( tc_core, test_file_ftruncate_shrink_cluster );
  tcase_add_test( tc_core, test_file_ftruncate_extend_size_only );
  tcase_add_test( tc_core, test_file_ftruncate_shrink_size_only );
  suite_add_tcase( s, tc_core );
  return s;
}
