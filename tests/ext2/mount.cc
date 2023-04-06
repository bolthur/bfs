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
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "../_helper.hh"
#include "gtest/gtest.h"

TEST( ext2, mount_readonly ) {
  helper_mount_ext_test_image( true, "ext2.img", "ext2", "/ext2/" );
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}

TEST( ext2, mount_read_write ) {
  helper_mount_ext_test_image( false, "ext2.img", "ext2", "/ext2/" );
  helper_unmount_ext_test_image( "ext2", "/ext2/" );
}
