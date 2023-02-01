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

#include <ctype.h>
#include <string.h>
#include <common/util.h>
#include <common/bfscommon_export.h>

/**
 * @brief Helper to trim trailing and leading whitespaces
 *
 * @param str
 * @return char*
 */
BFSCOMMON_EXPORT char* util_trim( char* str ) {
  char* end;
  // trim leading space
  while( isspace( ( int )*str ) ) {
    str++;
  }
  // handle end reached
  if ( 0 == *str ) {
    return str;
  }
  // trim trailing space
  end = str + strlen( str ) - 1;
  while( end > str && isspace( ( int )*end ) ) {
    end--;
  }
  // write new null terminator character
  end[1] = '\0';
  // return string
  return str;
}
