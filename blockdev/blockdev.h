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

/** @file blockdev/blockdev.h */

#include <common/blockdev.h>

#ifndef _BLOCKDEV_H
#define _BLOCKDEV_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined( _BFS_COMPILING )
  int blockdev_open( common_blockdev_t *bdev );
  int blockdev_read( common_blockdev_t *bdev, void *buf, uint64_t blk_id, uint64_t blk_cnt );
  int blockdev_write( common_blockdev_t *bdev, const void *buf, uint64_t blk_id, uint64_t blk_cnt );
  int blockdev_close( common_blockdev_t *bdev );
  int blockdev_lock( common_blockdev_t *bdev );
  int blockdev_unlock( common_blockdev_t *bdev );
  int blockdev_resize( common_blockdev_t* bdev, uint64_t block_size );
#endif

common_blockdev_t* common_blockdev_get(void);

#ifdef __cplusplus
}
#endif

#endif
