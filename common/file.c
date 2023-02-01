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
#include <fcntl.h>
#include <string.h>
#include <common/file.h>
#include <common/errno.h>
#include <common/bfscommon_export.h>

/**
 * @brief Helper to parse open flags
 *
 * @param flags
 * @param file_flags
 * @return int
 */
BFSCOMMON_EXPORT int common_file_parse_flags( const char* flags, int *file_flags ) {
  if ( ! flags || ! file_flags ) {
    return EINVAL;
  }
  // handle read
  if ( ! strcmp( flags, "r" ) || ! strcmp( flags, "rb" ) ) {
    *file_flags = O_RDONLY;
    return EOK;
  }
  // handle write
  if ( ! strcmp( flags, "w" ) || ! strcmp( flags, "wb" ) ) {
    *file_flags = O_WRONLY | O_CREAT | O_TRUNC;
    return EOK;
  }
  // handle append
  if ( ! strcmp( flags, "a" ) || ! strcmp( flags, "ab" ) ) {
    *file_flags = O_WRONLY | O_CREAT | O_APPEND;
    return EOK;
  }
  // handle r+
  if (
    ! strcmp( flags, "r+" )
    || ! strcmp( flags, "rb+" )
    || ! strcmp( flags, "r+b" )
  ) {
    *file_flags = O_RDWR;
    return EOK;
  }
  // handle w+
  if (
    ! strcmp( flags, "w+" )
    || ! strcmp( flags, "wb+" )
    || ! strcmp( flags, "w+b" )
  ) {
    *file_flags = O_RDWR | O_CREAT | O_TRUNC;
    return EOK;
  }
  // handle a+
  if (
    ! strcmp( flags, "a+" )
    || ! strcmp( flags, "ab+" )
    || ! strcmp( flags, "a+b" )
  ) {
    *file_flags = O_RDWR | O_CREAT | O_APPEND;
    return EOK;
  }
  // return invalid
  return EINVAL;
}
