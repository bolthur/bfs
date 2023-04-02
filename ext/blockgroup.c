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

#include <common/errno.h>
#include <ext/blockgroup.h>
#include <ext/superblock.h>

/**
 * @brief Helper to check whether block group contains a superblock
 *
 * @param superblock
 * @param blockgroup
 * @param result
 * @return int
 */
int blockgroup_has_superblock(
  ext_structure_superblock_t* superblock,
  uint64_t blockgroup,
  bool* result
) {
  // validate parameter
  if ( ! superblock || ! result ) {
    return EINVAL;
  }
  // check for sparse super
  if ( ! ( superblock->s_feature_ro_compat & EXT_SUPERBLOCK_EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER ) ) {
    *result = true;
    return EOK;
  }
  if ( 1 >= blockgroup ) {
    *result = true;
    return EOK;
  }
  if ( ! ( blockgroup & 1 ) ) {
    *result = false;
    return EOK;
  }
  *result = superblock_is_power_of( blockgroup, 7 )
    || superblock_is_power_of( blockgroup, 5 )
    || superblock_is_power_of( blockgroup, 3 );
  return EOK;
}
