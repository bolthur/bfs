// Copyright (C) 2022 - 2024 bolthur project.
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
#include <common/transaction.h>
#include <ext/mountpoint.h>
#include <ext/structure.h>
#include <ext/directory.h>
#include <ext/type.h>
#include <ext/iterator.h>
#include <ext/fs.h>
#include <ext/file.h>
#include "../_helper.hh"
#include "gtest/gtest.h"

TEST( ext2, transaction_nested_commit ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/ext2/" );
  EXPECT_TRUE( mp );
  // cache ext fs
  ext_fs_t* fs = ( ext_fs_t* )mp->fs;
  EXPECT_TRUE( fs );
  // start transaction
  int result = common_transaction_begin( fs->bdev );
  EXPECT_EQ( result, EOK );
  // fetch transaction
  common_transaction_t* t = transaction_get( fs->bdev );
  EXPECT_TRUE( t );
  EXPECT_EQ( t->counter, 1 );
  // start another transaction
  result = common_transaction_begin( fs->bdev );
  EXPECT_EQ( result, EOK );
  // fetch transaction again
  t = transaction_get( fs->bdev );
  EXPECT_TRUE( t );
  EXPECT_EQ( t->counter, 2 );
  // commit transaction
  result = common_transaction_commit( fs->bdev );
  EXPECT_EQ( result, EOK );
  // fetch transaction again
  t = transaction_get( fs->bdev );
  EXPECT_TRUE( t );
  EXPECT_EQ( t->counter, 1 );
  // commit second transaction
  result = common_transaction_commit( fs->bdev );
  EXPECT_EQ( result, EOK );
  // fetch transaction again
  t = transaction_get( fs->bdev );
  EXPECT_FALSE( t );
  // unmount test image again
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, transaction_nested_rollback ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  // get mountpoint
  common_mountpoint_t* mp = common_mountpoint_by_mountpoint( "/ext2/" );
  EXPECT_TRUE( mp );
  // cache ext fs
  ext_fs_t* fs = ( ext_fs_t* )mp->fs;
  EXPECT_TRUE( fs );
  // start transaction
  int result = common_transaction_begin( fs->bdev );
  EXPECT_EQ( result, EOK );
  // fetch transaction
  common_transaction_t* t = transaction_get( fs->bdev );
  EXPECT_TRUE( t );
  EXPECT_EQ( t->counter, 1 );
  // start another transaction
  result = common_transaction_begin( fs->bdev );
  EXPECT_EQ( result, EOK );
  // fetch transaction again
  t = transaction_get( fs->bdev );
  EXPECT_TRUE( t );
  EXPECT_EQ( t->counter, 2 );
  // commit transaction
  result = common_transaction_rollback( fs->bdev );
  EXPECT_EQ( result, EOK );
  // fetch transaction again
  t = transaction_get( fs->bdev );
  EXPECT_FALSE( t );
  // unmount test image again
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}
