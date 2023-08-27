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
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <libgen.h>
#include <sys/stat.h>
#include <common/mountpoint.h>
#include <common/errno.h> // IWYU pragma: keep
#include <fat/stat.h>
#include <fat/directory.h>
#include <fat/structure.h>
#include <fat/type.h>
#include <fat/bfsfat_export.h>
#include <bfsconfig.h>

BFSFAT_EXPORT int fat_stat( const char* path, struct stat *st ) {
  if ( ! path || ! st ) {
    return EINVAL;
  }
  // try to get mountpoint by path
  common_mountpoint_t* mp = common_mountpoint_find( path );
  if ( ! mp ) {
    return ENOENT;
  }
  // get real path without mountpoint
  const char* real_path = path + strlen( mp->name ) - 1;
  char* duppath = strdup( real_path );
  if ( ! duppath ) {
    return ENOMEM;
  }
  // duplicate path for base and dirname
  char* pathdup_dir = strdup( duppath );
  if ( ! pathdup_dir ) {
    free( duppath );
    return ENOMEM;
  }
  char* pathdup_base = strdup( duppath );
  if ( ! pathdup_base ) {
    free( pathdup_dir );
    free( duppath );
    return ENOMEM;
  }
  // get dir and base name
  char* dirpath = dirname( pathdup_dir );
  char* basepath = basename( pathdup_base );
  // check for unsupported
  if ( '.' == *dirpath ) {
    free( pathdup_base );
    free( pathdup_dir );
    free( duppath );
    return ENOTSUP;
  }
  size_t open_path_size = strlen( mp->name ) + strlen( dirpath ) + 1;
  char* open_path = malloc( open_path_size );
  if ( ! open_path ) {
    free( pathdup_base );
    free( pathdup_dir );
    free( duppath );
    return ENOMEM;
  }
  strcpy( open_path, mp->name );
  strcat( open_path, dirpath + 1 );
  // Add trailing slash if not existing, necessary, when opening root directory
  if ( CONFIG_PATH_SEPARATOR_CHAR != open_path[ strlen( open_path ) - 1 ] ) {
    strcat( open_path, CONFIG_PATH_SEPARATOR_STRING );
  }
  // open directory
  fat_directory_t dir;
  memset( &dir, 0, sizeof( dir ) );
  int result = fat_directory_open( &dir, open_path );
  if ( EOK != result ) {
    free( open_path );
    free( pathdup_base );
    free( pathdup_dir );
    free( duppath );
    return result;
  }
  // variables for access creation and modification time
  time_t atime = 0;
  time_t mtime = 0;
  time_t ctime = 0;
  mode_t mode = S_IFDIR;
  uint64_t size = 0;
  // handle valid base path
  if ( 0 != strcmp( dirpath, basepath ) ) {
    // get entry by name
    result = fat_directory_entry_by_name( &dir, basepath );
    if ( EOK != result ) {
      fat_directory_close( &dir );
      free( pathdup_base );
      free( pathdup_dir );
      free( duppath );
      return result;
    }
    // fetch access modification and creation time
    if (
      EOK != fat_directory_atime( &dir, &atime )
      || EOK != fat_directory_mtime( &dir, &mtime )
      || EOK != fat_directory_ctime( &dir, &ctime )
    ) {
      fat_directory_close( &dir );
      free( pathdup_base );
      free( pathdup_dir );
      free( duppath );
      return result;
    }
    // set mode and size
    size = dir.entry->file_size;
    mode = 0 == size ? S_IFDIR : S_IFREG;
  }
  // close directory again
  result = fat_directory_close( &dir );
  if ( EOK != result ) {
    free( pathdup_base );
    free( pathdup_dir );
    free( duppath );
    return result;
  }
  free( open_path );
  // fill stat struct
  st->st_dev = 0;
  st->st_ino = 0; // inode_number;
  st->st_mode = mode; // inode.i_mode;
  st->st_nlink = 0; // inode.i_links_count;
  st->st_uid = 0; // inode.i_uid;
  st->st_gid = 0; // inode.i_gid;
  st->st_rdev = 0;
  st->st_size = ( off_t )size; // inode.i_size;
  st->st_atim.tv_sec = atime; // inode.i_atime;
  st->st_atim.tv_nsec = 0;
  st->st_mtim.tv_sec = mtime; // inode.i_mtime;
  st->st_mtim.tv_nsec = 0;
  st->st_ctim.tv_sec = ctime; // inode.i_ctime;
  st->st_ctim.tv_nsec = 0;
  st->st_blksize = 0; // ( blksize_t )( ( fat_fs_t* )mp->fs)->bdev->bdif->block_size;
  st->st_blocks = 0; // ( blkcnt_t )inode.i_blocks;
  // free duplicates
  free( pathdup_base );
  free( pathdup_dir );
  free( duppath );
  // return success
  return EOK;
}
