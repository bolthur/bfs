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

/** @file fat/log.h */

#ifndef _FAT_LOG_H
#define _FAT_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined( _BFS_COMPILING )

#if 1 != CONFIG_ENABLE_LOGGING
#define fat_log( ... ) {}
#else
#if defined(__bolthur__)
#include <sys/bolthur.h>
#define fat_log EARLY_STARTUP_PRINT
#else
void fat_log( const char* msg, ... );
#endif
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
