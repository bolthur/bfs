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

/** @file common/errno.h */

#ifndef _COMMON_ERRNO_H
#define _COMMON_ERRNO_H

#include <bfsconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_HAVE_ERRNO_H
  #include <errno.h>
#else
  /** @brief Fallback for ENOENT, when no errno is available */
  #define ENOENT 2
  /** @brief Fallback for EIO, when no errno is available */
  #define EIO 5
  /** @brief Fallback for ENOMEM, when no errno is available */
  #define ENOMEM 12
  /** @brief Fallback for EFAULT, when no errno is available */
  #define EFAULT 14
  /** @brief Fallback for EEXIST, when no errno is available */
  #define EEXIST 17
  /** @brief Fallback for ENODEV, when no errno is available */
  #define ENODEV 19
  /** @brief Fallback for EINVAL, when no errno is available */
  #define EINVAL 22
  /** @brief Fallback for ENODATA, when no errno is available */
  #define ENODATA 61
  /** @brief Fallback for ENOSYS, when no errno is available */
  #define ENOSYS 88
  /** @brief Fallback for ENOTSUP, when no errno is available */
  #define ENOTSUP 134
#endif

/** @brief Successful return value */
#define EOK 0

#ifdef __cplusplus
}
#endif

#endif
