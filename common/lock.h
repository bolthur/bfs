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

/** @file common/lock.h */

#ifndef _COMMON_LOCK_H
#define _COMMON_LOCK_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Common lock interface */
typedef struct {
  /** @brief Method to perform lock */
  void ( *lock )( void );
  /** @brief Method to perform unlock */
  void ( *unlock )( void );
} common_lock_t;

#ifdef __cplusplus
}
#endif

#endif
