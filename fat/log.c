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

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <fat/log.h>
#include <fat/bfsfat_export.h>

#if ! defined(__bolthur__) && 1 == CONFIG_ENABLE_LOGGING
BFSFAT_NO_EXPORT __attribute__( ( format( printf, 1, 2 ) ) ) void fat_log( const char* msg, ... ) {
  // va list pointer
  va_list ptr;
  // start arguments
  va_start( ptr, msg );
  // printf
  vprintf( msg, ptr );
  // end arguments
  va_end( ptr );
}
#endif
