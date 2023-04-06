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
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, root_directory_read_dir_utils ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, root_directory_read_dir_utils_rewind ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, directory_get_by_name ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  /// FIXME: ADD LOGIC
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
