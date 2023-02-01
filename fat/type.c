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
#include <common/errno.h>
#include <fat/fs.h>
#include <fat/type.h>
#include <fat/structure.h>
#include <fat/bfsfat_export.h>

/**
 * @brief Extract fat type
 *
 * @param fs
 * @return int
 */
BFSFAT_NO_EXPORT int fat_type_validate( fat_fs_t* fs ) {
  // exfat
  if ( 0 == fs->superblock.bytes_per_sector ) {
    return ENOTSUP;
  // fat12
  } else if ( 4085 > fs->total_clusters ) {
    fs->type = FAT_FAT12;
  // fat16 / vfat
  } else if ( 65525 > fs->total_clusters ) {
    fs->type = FAT_FAT16;
  // fat32
  } else {
    fs->type = FAT_FAT32;
  }
  // return success
  return EOK;
}
