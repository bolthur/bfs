// Copyright (C) 2022 - 2026 bolthur project.
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

/** @file ext/structure.h */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#ifndef _EXT_STRUCTURE_H
#define _EXT_STRUCTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

/** @brief super block magic */
#define EXT_SUPERBLOCK_MAGIC 0xEF53

#define EXT_GOOD_OLD_REV 0
#define EXT_DYNAMIC_REV 1

// s_state
/** @brief super block valid filesystem */
#define EXT_SUPERBLOCK_EXT2_VALID_FS 1
/** @brief super block error file system */
#define EXT_SUPERBLOCK_EXT2_ERROR_FS 2
// s_errors
#define EXT_SUPERBLOCK_EXT2_ERRORS_CONTINUE 1
#define EXT_SUPERBLOCK_EXT2_ERRORS_RO 2
#define EXT_SUPERBLOCK_EXT2_ERRORS_PANIC 3
// s_creator_os
#define EXT_SUPERBLOCK_EXT2_OS_LINUX 0
#define EXT_SUPERBLOCK_EXT2_OS_HURD 1
#define EXT_SUPERBLOCK_EXT2_OS_MASIX 2
#define EXT_SUPERBLOCK_EXT2_OS_FREEBSD 3
#define EXT_SUPERBLOCK_EXT2_OS_LITES 4
// s_rev_level
#define EXT_SUPERBLOCK_EXT2_GOOD_OLD_REV 0
#define EXT_SUPERBLOCK_EXT2_DYNAMIC_REV 1
// s_feature_compat
#define EXT_SUPERBLOCK_EXT2_FEATURE_COMPAT_DIR_PREALLOC 0x0001
#define EXT_SUPERBLOCK_EXT2_FEATURE_COMPAT_IMAGIC_INODES 0x0002
#define EXT_SUPERBLOCK_EXT3_FEATURE_COMPAT_HAS_JOURNAL 0x0004
#define EXT_SUPERBLOCK_EXT2_FEATURE_COMPAT_EXT_ATTR 0x0008
#define EXT_SUPERBLOCK_EXT2_FEATURE_COMPAT_RESIZE_INO 0x0010
#define EXT_SUPERBLOCK_EXT2_FEATURE_COMPAT_DIR_INDEX 0x0020
// s_feature_incompat
#define EXT_SUPERBLOCK_EXT2_FEATURE_INCOMPAT_COMPRESSION 0x0001
#define EXT_SUPERBLOCK_EXT2_FEATURE_INCOMPAT_FILETYPE 0x0002
#define EXT_SUPERBLOCK_EXT3_FEATURE_INCOMPAT_RECOVER 0x0004
#define EXT_SUPERBLOCK_EXT3_FEATURE_INCOMPAT_JOURNAL_DEV 0x0008
#define EXT_SUPERBLOCK_EXT2_FEATURE_INCOMPAT_META_BG 0x0010
// s_feature_ro_compat
#define EXT_SUPERBLOCK_EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER 0x0001
#define EXT_SUPERBLOCK_EXT2_FEATURE_RO_COMPAT_LARGE_FILE 0x0002
#define EXT_SUPERBLOCK_EXT2_FEATURE_RO_COMPAT_BTREE_DIR 0x0004
// s_algo_bitmap
#define EXT_SUPERBLOCK_EXT2_LZV1_ALG 0
#define EXT_SUPERBLOCK_EXT2_LZRW3A_ALG 1
#define EXT_SUPERBLOCK_EXT2_GZIP_ALG 2
#define EXT_SUPERBLOCK_EXT2_BZIP2_ALG 3
#define EXT_SUPERBLOCK_EXT2_LZO_ALG 4

typedef struct {
  /** @brief inode count */
  uint32_t s_inodes_count;
  /** @brief block count */
  uint32_t s_blocks_count;
  /** @brief reserved blocks count */
  uint32_t s_r_blocks_count;
  /** @brief free blocks count */
  uint32_t s_free_blocks_count;
  /** @brief free inode count */
  uint32_t s_free_inodes_count;
  /** @brief first data block */
  uint32_t s_first_data_block;
  /** @brief block size */
  uint32_t s_log_block_size;
  /** @brief frag size */
  uint32_t s_log_frag_size;
  /** @brief amount of blocks per group */
  uint32_t s_blocks_per_group;
  /** @brief amount of frags per group */
  uint32_t s_frags_per_group;
  /** @brief amount of inodes per group */
  uint32_t s_inodes_per_group;
  /** @brief mtime */
  uint32_t s_mtime;
  /** @brief wtime */
  uint32_t s_wtime;
  /** @brief mount count */
  uint16_t s_mnt_count;
  /** @brief max mount count */
  uint16_t s_max_mnt_count;
  /** @brief magix */
  uint16_t s_magic;
  /** @brief state */
  uint16_t s_state;
  /** @brief errors */
  uint16_t s_errors;
  /** @brief minor revision */
  uint16_t s_minor_rev_level;
  /** @brief last check */
  uint32_t s_lastcheck;
  /** @brief check interval */
  uint32_t s_checkinterval;
  /** @brief creator os */
  uint32_t s_creator_os;
  /** @brief revision */
  uint32_t s_rev_level;
  /** @brief resuid */
  uint16_t s_def_resuid;
  /** @brief resgid */
  uint16_t s_def_resgid;
  // only for ext greater or equal to 1
  /** @brief first inode */
  uint32_t s_first_ino;
  /** @brief inode size */
  uint16_t s_inode_size;
  /** @brief block group number */
  uint16_t s_block_group_nr;
  /** @brief feature compatibility */
  uint32_t s_feature_compat;
  /** @brief feature incompatibility */
  uint32_t s_feature_incompat;
  /** @brief read only feature compatibility */
  uint32_t s_feature_ro_compat;
  /** @brief uuid */
  uint8_t s_uuid[ 16 ];
  /** @brief columen name */
  uint8_t s_volume_name[ 16 ];
  /** @brief last mounted */
  uint8_t s_last_mounted[ 64 ];
  /** @brief bitmap algorithm */
  uint32_t s_algo_bitmap;
  // performance hints
  /** @brief pre allocated blocks */
  uint8_t s_prealloc_blocks;
  /** @brief pre allocated directory blocks */
  uint8_t s_prealloc_dir_blocks;
  /** @brief alignment */
  uint16_t alignment0;
  // journaling
  /** @brief journal uuid */
  uint8_t s_journal_uuid[ 16 ];
  /** @brief journal inum */
  uint32_t s_journal_inum;
  /** @brief journal dev */
  uint32_t s_journal_dev;
  /** @brief last orphan */
  uint32_t s_last_orphan;
  // directory index
  /** @brief hash seed */
  uint32_t s_hash_seed[ 4 ];
  /** @brief hash version */
  uint8_t s_def_hash_version;
  /** @brief padding */
  uint8_t padding0[ 3 ];
  // other options
  /** @brief default mount options */
  uint32_t s_default_mount_options;
  /** @brief first meta block group */
  uint32_t s_first_meta_bg;
  /** @brief unused */
  uint8_t unused0[ 760 ];
} ext_structure_superblock_t;
static_assert(
  1024 == sizeof( ext_structure_superblock_t ),
  "invalid ext_structure_superblock_t size!"
);

/** @brief block group descriptor structure */
typedef struct {
  /** @brief block bitmap */
  uint32_t bg_block_bitmap;
  /** @brief inode bitmap */
  uint32_t bg_inode_bitmap;
  /** @brief inode table */
  uint32_t bg_inode_table;
  /** @brief free block count */
  uint16_t bg_free_blocks_count;
  /** @brief free inode count */
  uint16_t bg_free_inodes_count;
  /** @brief used directories count */
  uint16_t bg_used_dirs_count;
  /** @brief padding */
  uint16_t bg_pad;
  /** @brief reserved */
  uint8_t bg_reserved[ 12 ];
} ext_structure_block_group_descriptor_t;
static_assert(
  32 == sizeof( ext_structure_block_group_descriptor_t ),
  "invalid ext_structure_block_group_descriptor_t size!"
);

// reserved inodes
/** @brief bad inode value */
#define EXT_INODE_EXT2_BAD_INO 1
/** @brief root inode */
#define EXT_INODE_EXT2_ROOT_INO 2
/** @brief acl index inode */
#define EXT_INODE_EXT2_ACL_IDX_INO 3
/** @brief acl data inode */
#define EXT_INODE_EXT2_ACL_DATA_INO 4
/** @brief boot loader inode */
#define EXT_INODE_EXT2_BOOT_LOADER_INO 5
/** @brief undel directory inode */
#define EXT_INODE_EXT2_UNDEL_DIR_INO 6

// mode values
#define EXT_INODE_EXT2_S_IFSOCK 0xC000
#define EXT_INODE_EXT2_S_IFLNK  0xA000
#define EXT_INODE_EXT2_S_IFREG  0x8000
#define EXT_INODE_EXT2_S_IFBLK  0x6000
#define EXT_INODE_EXT2_S_IFDIR  0x4000
#define EXT_INODE_EXT2_S_IFCHR  0x2000
#define EXT_INODE_EXT2_S_IFIFO  0x1000
// process execution user/group override
#define EXT_INODE_EXT2_S_ISUID  0x0800
#define EXT_INODE_EXT2_S_ISGID  0x0400
#define EXT_INODE_EXT2_S_ISVTX  0x0200
// access rights
#define EXT_INODE_EXT2_S_IRUSR  0x0100
#define EXT_INODE_EXT2_S_IWUSR  0x0080
#define EXT_INODE_EXT2_S_IXUSR  0x0040
#define EXT_INODE_EXT2_S_IRGRP  0x0020
#define EXT_INODE_EXT2_S_IWGRP  0x0010
#define EXT_INODE_EXT2_S_IXGRP  0x0008
#define EXT_INODE_EXT2_S_IROTH  0x0004
#define EXT_INODE_EXT2_S_IWOTH  0x0002
#define EXT_INODE_EXT2_S_IXOTH  0x0001

// i_flags
#define EXT_INODE_EXT2_SECRM_FL 0x00000001
#define EXT_INODE_EXT2_UNRM_FL  0x00000002
#define EXT_INODE_EXT2_COMPR_FL 0x00000004
#define EXT_INODE_EXT2_SYNC_FL  0x00000008
#define EXT_INODE_EXT2_IMMUTABLE_FL 0x00000010
#define EXT_INODE_EXT2_APPEND_FL  0x00000020
#define EXT_INODE_EXT2_NODUMP_FL  0x00000040
#define EXT_INODE_EXT2_NOATIME_FL 0x00000080
// Reserved for compression usage
#define EXT_INODE_EXT2_DIRTY_FL 0x00000100
#define EXT_INODE_EXT2_COMPRBLK_FL  0x00000200
#define EXT_INODE_EXT2_NOCOMPR_FL 0x00000400
#define EXT_INODE_EXT2_ECOMPR_FL  0x00000800
// End of compression flags
#define EXT_INODE_EXT2_BTREE_FL 0x00001000
#define EXT_INODE_EXT2_INDEX_FL 0x00001000
#define EXT_INODE_EXT2_IMAGIC_FL  0x00002000
#define EXT_INODE_EXT3_JOURNAL_DATA_FL  0x00004000
#define EXT_INODE_EXT2_RESERVED_FL  0x80000000

/** @brief inode structure */
typedef struct {
  /** @brief mode */
  uint16_t i_mode;
  /** @brief uid */
  uint16_t i_uid;
  /** @brief size */
  uint32_t i_size;
  /** @brief atime */
  uint32_t i_atime;
  /** @brief ctime */
  uint32_t i_ctime;
  /** @brief mtime */
  uint32_t i_mtime;
  /** @brief dtime */
  uint32_t i_dtime;
  /** @brief gid */
  uint16_t i_gid;
  /** @brief link count */
  uint16_t i_links_count;
  /** @brief block count*/
  uint32_t i_blocks;
  /** @brief flags */
  uint32_t i_flags;
  /** @brief os dependant 1 */
  uint32_t i_osd1;
  /** @brief blocks */
  uint32_t i_block[ 15 ];
  /** @brief generation */
  uint32_t i_generation;
  /** @brief file acl */
  uint32_t i_file_acl;
  /** @brief dir acl */
  uint32_t i_dir_acl;
  /** @brief faddr */
  uint32_t i_faddr;
  /** @brief os information */
  union {
    /** @brief hurd related */
    struct {
      /** @brief frag */
      uint8_t h_i_frag;
      /** @brief size */
      uint8_t h_i_fsize;
      /** @brief mode high */
      uint16_t h_i_mode_high;
      /** @brief uid high */
      uint16_t h_i_uid_high;
      /** @brief gid high */
      uint16_t h_i_gid_high;
      /** @brief author */
      uint32_t h_i_author;
    } i_osd2_hurd;
    /** @brief linux related */
    struct {
      /** @brief frag */
      uint8_t h_i_frag;
      /** @brief size */
      uint8_t h_i_fsize;
      /** @brief reserved */
      uint16_t reserved0;
      /** @brief high uid */
      uint16_t h_i_uid_high;
      /** @brief high gid */
      uint16_t h_i_gid_high;
      /** @brief reserved */
      uint32_t reserved1;
    } i_osd2_linux;
    /** @brief masix related */
    struct {
      /** @brief frag */
      uint8_t h_i_frag;
      /** @brief size */
      uint8_t h_i_fsize;
      /** @brief reserved */
      uint8_t reserved0[ 10 ];
    } i_osd2_masix;
    /** @brief os dependant 2 */
    uint8_t i_osd2[ 12 ];
  } os_information;
} ext_structure_inode_t;
static_assert(
  128 == sizeof( ext_structure_inode_t ),
  "invalid ext_structure_inode_t size!"
);

/** @brief directory entry file type for unknown */
#define EXT_DIRECTORY_EXT2_FT_UNKNOWN 0
/** @brief directory entry file type for file */
#define EXT_DIRECTORY_EXT2_FT_REG_FILE 1
/** @brief directory entry file type for directory */
#define EXT_DIRECTORY_EXT2_FT_DIR 2
/** @brief directory entry file type for character device */
#define EXT_DIRECTORY_EXT2_FT_CHRDEV 3
/** @brief directory entry file type for block device */
#define EXT_DIRECTORY_EXT2_FT_BLKDEV 4
/** @brief directory entry file type for fifo */
#define EXT_DIRECTORY_EXT2_FT_FIFO 5
/** @brief directory entry file type for socket */
#define EXT_DIRECTORY_EXT2_FT_SOCK 6
/** @brief directory entry file type for symlink */
#define EXT_DIRECTORY_EXT2_FT_SYMLINK 7

/** @brief ext directory entry representation */
typedef struct {
  /** @brief inode number */
  uint32_t inode;
  /** @brief record length */
  uint16_t rec_len;
  /** @brief name length */
  uint8_t name_len;
  /** @brief file type */
  uint8_t file_type;
  /** @brief entry name */
#ifdef __cplusplus
  char name[255];
#else
  char name[];
#endif
} ext_structure_directory_entry_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
