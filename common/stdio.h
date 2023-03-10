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

/** @file common/stdio.h */

#include <bfsconfig.h>

#ifndef _COMMON_STDIO_H
#define _COMMON_STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_HAVE_STDIO_H
  #include <stdio.h>
#else
  /** @brief Fallback for FILENAME_MAX, when no stdio is available */
  #define FILENAME_MAX 1024
  /** @brief Fallback for SEEK_SET, when no stdio is available */
  #define SEEK_SET 0
  /** @brief Fallback for SEEK_CUR, when no stdio is available */
  #define SEEK_CUR 1
  /** @brief Fallback for SEEK_END, when no stdio is available */
  #define SEEK_END 2
#endif

#ifdef __cplusplus
}
#endif

#endif
