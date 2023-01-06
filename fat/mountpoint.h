/**
 * Copyright (C) 2022 bolthur project.
 *
 * This file is part of bolthur/bfs.
 *
 * bolthur/bfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bolthur/bfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bolthur/bfs.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

#ifndef _FAT_MOUNTPOINT_H
#define _FAT_MOUNTPOINT_H

#ifdef __cplusplus
extern "C" {
#endif

int fat_mountpoint_mount( const char* device_name, const char* mountpoint, bool read_only );
int fat_mountpoint_umount( const char* mountpoint );

#ifdef __cplusplus
}
#endif

#endif
