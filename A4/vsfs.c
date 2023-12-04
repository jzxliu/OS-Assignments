/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC369H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * Authors: Alexey Khrabrov, Karen Reid, Angela Demke Brown
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2022 Angela Demke Brown
 */

/**
 * CSC369 Assignment 4 - vsfs driver implementation.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

// Using 2.9.x FUSE API
#define FUSE_USE_VERSION 29
#include <fuse.h>

#include "vsfs.h"
#include "fs_ctx.h"
#include "options.h"
#include "util.h"
#include "bitmap.h"
#include "map.h"

//NOTE: All path arguments are absolute paths within the vsfs file system and
// start with a '/' that corresponds to the vsfs root directory.
//
// For example, if vsfs is mounted at "/tmp/my_userid", the path to a
// file at "/tmp/my_userid/dir/file" (as seen by the OS) will be
// passed to FUSE callbacks as "/dir/file".
//
// Paths to directories (except for the root directory - "/") do not end in a
// trailing '/'. For example, "/tmp/my_userid/dir/" will be passed to
// FUSE callbacks as "/dir".


/**
 * Initialize the file system.
 *
 * Called when the file system is mounted. NOTE: we are not using the FUSE
 * init() callback since it doesn't support returning errors. This function must
 * be called explicitly before fuse_main().
 *
 * @param fs    file system context to initialize.
 * @param opts  command line options.
 * @return      true on success; false on failure.
 */
static bool vsfs_init(fs_ctx *fs, vsfs_opts *opts)
{
	size_t size;
	void *image;
	
	// Nothing to initialize if only printing help
	if (opts->help) {
		return true;
	}

	// Map the disk image file into memory
	image = map_file(opts->img_path, VSFS_BLOCK_SIZE, &size);
	if (image == NULL) {
		return false;
	}

	return fs_ctx_init(fs, image, size);
}

/**
 * Cleanup the file system.
 *
 * Called when the file system is unmounted. Must cleanup all the resources
 * created in vsfs_init().
 */
static void vsfs_destroy(void *ctx)
{
	fs_ctx *fs = (fs_ctx*)ctx;
	if (fs->image) {
		munmap(fs->image, fs->size);
		fs_ctx_destroy(fs);
	}
}

/** Get file system context. */
static fs_ctx *get_fs(void)
{
	return (fs_ctx*)fuse_get_context()->private_data;
}


/* Returns stores the inode number for the element at the end of the path to the pointer pointed by ino if it exists.
 * Returns 0 if successful. If there is any error, return -1.
 * Possible errors include:
 *   - The path is not an absolute path
 *   - An element on the path cannot be found
 */
static int path_lookup(const char *path,  vsfs_ino_t *ino) {
	if(path[0] != '/') {
		fprintf(stderr, "Not an absolute path\n");
		return -ENOTDIR;
	}

	if (strcmp(path, "/") == 0) {
		*ino = VSFS_ROOT_INO;
		return 0;
	}

    // Since only one directory (root dir), no need to do parsing yay
    fs_ctx *fs = get_fs();
    vsfs_inode *root_ino = &fs->itable[VSFS_ROOT_INO];

    // Search in direct entries first
    for (int n = 0; n < VSFS_NUM_DIRECT; n++) {
        if (root_ino->i_direct[n] >= fs->sb->sb_data_region && root_ino->i_direct[n] < VSFS_BLK_MAX) {
            vsfs_dentry *direct_entries = (vsfs_dentry *)(fs->image + root_ino->i_direct[n] * VSFS_BLOCK_SIZE);
            for (size_t i = 0; i < root_ino->i_size / sizeof(vsfs_dentry); i++) {
                if (strcmp(direct_entries[i].name, path + 1) == 0) {
                    *ino = direct_entries[i].ino;
                    return 0;
                }
            }
        }
    }

    // Search in indirect entries if it exists
    if (root_ino->i_indirect >= fs->sb->sb_data_region && root_ino->i_indirect < VSFS_BLK_MAX){
        vsfs_dentry *indirect_entries = (vsfs_dentry *)(fs->image + root_ino->i_indirect * VSFS_BLOCK_SIZE);
        for (size_t i = 0; i < root_ino->i_size / sizeof(vsfs_dentry); i++) {
            if (strcmp(indirect_entries[i].name, path + 1) == 0) {
                *ino = indirect_entries[i].ino;
                return 0;
            }
        }
    }

	return -ENOENT; // Not found
}

/**
 * Get file system statistics.
 *
 * Implements the statvfs() system call. See "man 2 statvfs" for details.
 * The f_bfree and f_bavail fields should be set to the same value.
 * The f_ffree and f_favail fields should be set to the same value.
 * The following fields can be ignored: f_fsid, f_flag.
 * All remaining fields are required.
 *
 * Errors: none
 *
 * @param path  path to any file in the file system. Can be ignored.
 * @param st    pointer to the struct statvfs that receives the result.
 * @return      0 on success; -errno on error.
 */
static int vsfs_statfs(const char *path, struct statvfs *st)
{
	(void)path;// unused
	fs_ctx *fs = get_fs();
	vsfs_superblock *sb = fs->sb; /* Get ptr to superblock from context */

	memset(st, 0, sizeof(*st));
	st->f_bsize   = VSFS_BLOCK_SIZE;   /* Filesystem block size */
	st->f_frsize  = VSFS_BLOCK_SIZE;   /* Fragment size */
	// The rest of required fields are filled based on the information
	// stored in the superblock.
        st->f_blocks = sb->sb_num_blocks;     /* Size of fs in f_frsize units */
        st->f_bfree  = sb->sb_free_blocks;    /* Number of free blocks */
        st->f_bavail = sb->sb_free_blocks;    /* Free blocks for unpriv users */
	st->f_files  = sb->sb_num_inodes;     /* Number of inodes */
        st->f_ffree  = sb->sb_free_inodes;    /* Number of free inodes */
        st->f_favail = sb->sb_free_inodes;    /* Free inodes for unpriv users */

	st->f_namemax = VSFS_NAME_MAX;     /* Maximum filename length */

	return 0;
}

/**
 * Get file or directory attributes.
 *
 * Implements the lstat() system call. See "man 2 lstat" for details.
 * The following fields can be ignored: st_dev, st_ino, st_uid, st_gid, st_rdev,
 *                                      st_blksize, st_atim, st_ctim.
 * All remaining fields are required.
 *
 * NOTE: the st_blocks field is measured in 512-byte units (disk sectors);
 *       it should include any metadata blocks that are allocated to the
 *       inode (for vsfs, that is the indirect block).
 *
 * NOTE2: the st_mode field must be set correctly for files and directories.
 *
 * Errors:
 *   ENAMETOOLONG  the path or one of its components is too long.
 *   ENOENT        a component of the path does not exist.
 *   ENOTDIR       a component of the path prefix is not a directory.
 *
 * @param path  path to a file or directory.
 * @param st    pointer to the struct stat that receives the result.
 * @return      0 on success; -errno on error;
 */
static int vsfs_getattr(const char *path, struct stat *st)
{
	if (strlen(path) >= VSFS_PATH_MAX) return -ENAMETOOLONG;
	fs_ctx *fs = get_fs();

	memset(st, 0, sizeof(*st));

//	//NOTE: This is just a placeholder that allows the file system to be
//	//      mounted without errors.
//	//      You should remove this from your implementation.
//	if (strcmp(path, "/") == 0) {
//		//NOTE: all the fields set below are required and must be set
//		// using the information stored in the corresponding inode
//		st->st_ino = 0;
//		st->st_mode = S_IFDIR | 0777;
//		st->st_nlink = 2;
//		st->st_size = 0;
//		st->st_blocks = 0 * VSFS_BLOCK_SIZE / 512;
//		st->st_mtim = (struct timespec){0};
//		return 0;
//	}

    vsfs_ino_t ino;
    int ret = path_lookup(path, &ino);
    if (ret) { // Path lookup did not succeed
        return ret; // Return the respective error code
    }
    vsfs_inode *inode = &fs->itable[ino];
    st->st_ino = ino;
    st->st_mode = inode->i_mode;
    st->st_nlink = inode->i_nlink;
    st->st_size = inode->i_size;
    st->st_blocks = inode->i_blocks * (VSFS_BLOCK_SIZE / 512); // in 512-byte units
    if (inode->i_indirect >= fs->sb->sb_data_region && inode->i_indirect < VSFS_BLK_MAX) { // Valid indirect index
        // Count an extra indirect block
        st->st_blocks += (VSFS_BLOCK_SIZE / 512);
    }
    st->st_mtim = inode->i_mtime;

    return 0;
}

/**
 * Read a directory.
 *
 * Implements the readdir() system call. Should call filler(buf, name, NULL, 0)
 * for each directory entry. See fuse.h in libfuse source code for details.
 *
 * Assumptions (already verified by FUSE using getattr() calls):
 *   "path" exists and is a directory.
 *
 * Errors:
 *   ENOMEM  not enough memory (e.g. a filler() call failed).
 *   ENOTDIR if path is not the root directory.
 *
 * @param path    path to the directory.
 * @param buf     buffer that receives the result.
 * @param filler  function that needs to be called for each directory entry.
 *                Pass 0 as offset (4th argument). 3rd argument can be NULL.
 * @param offset  unused.
 * @param fi      unused.
 * @return        0 on success; -errno on error.
 */
static int vsfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi)
{
	(void)offset;// unused
	(void)fi;// unused
    fs_ctx *fs = get_fs();
    vsfs_inode *ino = &fs->itable[VSFS_ROOT_INO];

    if (strcmp(path, "/") != 0) {
        return -ENOTDIR; // VSFS only has the root directory, so we dont have to implement for other cases
    }

    vsfs_dentry *root_entries = (vsfs_dentry *)(fs->image + ino->i_direct[0] * VSFS_BLOCK_SIZE);
    // Iterate through every entry in root directory
    for (size_t i = 0; i < ino->i_size / sizeof(vsfs_dentry); i++) {
        if (root_entries[i].ino != VSFS_INO_MAX) {
            if (filler(buf, root_entries[i].name, NULL, 0)) {
                return -ENOMEM;
            }
        }
    }

    return 0;
}


/**
 * Create a file.
 *
 * Implements the open()/creat() system call.
 *
 * Assumptions (already verified by FUSE using getattr() calls):
 *   "path" doesn't exist.
 *   The parent directory of "path" exists and is a directory.
 *   "path" and its components are not too long.
 *
 * Errors:
 *   ENOMEM  not enough memory (e.g. a malloc() call failed).
 *   ENOSPC  not enough free space in the file system.
 *
 * @param path  path to the file to create.
 * @param mode  file mode bits.
 * @param fi    unused.
 * @return      0 on success; -errno on error.
 */
static int vsfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	(void)fi;// unused
	assert(S_ISREG(mode));
	fs_ctx *fs = get_fs();
    vsfs_inode *root_ino = &fs->itable[VSFS_ROOT_INO];
    vsfs_dentry *root_entries = (vsfs_dentry *)(fs->image + root_ino->i_direct[0] * VSFS_BLOCK_SIZE);

    unsigned int index;
    int ret = bitmap_alloc(fs->ibmap, fs->sb->sb_num_inodes, &index);
    if (ret) { // No free inodes
        return -ENOSPC;
    }

    // Create a new inode
    vsfs_inode *new_inode = &fs->itable[index];
    new_inode->i_mode = S_IFREG | mode;
    new_inode->i_nlink = 1;
    new_inode->i_size = 0;
    new_inode->i_blocks = 0;
    fs->sb->sb_free_inodes -= 1;

    // Find a space in root directory to put new inode
    for (size_t i = 0; i < root_ino->i_size / sizeof(vsfs_dentry); i++) {
        if (root_entries[i].ino == VSFS_INO_MAX) {
            root_entries[i].ino = index;
            strncpy(root_entries[i].name, path + 1, VSFS_NAME_MAX - 1); // Does not copy the '/'
            root_ino->i_nlink += 1;
            return 0;
        }
    }

    // No free space in root directory
    bitmap_free(fs->ibmap, fs->sb->sb_num_inodes, index);
    fs->sb->sb_free_inodes += 1;
    return -ENOSPC;
}

/**
 * Remove a file.
 *
 * Implements the unlink() system call.
 *
 * Assumptions (already verified by FUSE using getattr() calls):
 *   "path" exists and is a file.
 *
 * Errors: none
 *
 * @param path  path to the file to remove.
 * @return      0 on success; -errno on error.
 */
static int vsfs_unlink(const char *path)
{
	fs_ctx *fs = get_fs();
    vsfs_inode *root_ino = &fs->itable[VSFS_ROOT_INO];
    vsfs_dentry *root_entries = (vsfs_dentry *)(fs->image + root_ino->i_direct[0] * VSFS_BLOCK_SIZE);
    fs->sb->sb_free_inodes += 1;

    // Look for the file in root directory
    for (size_t i = 0; i < root_ino->i_size / sizeof(vsfs_dentry); i++) {
        if (strcmp(root_entries[i].name, path + 1) == 0) {
            bitmap_free(fs->ibmap, fs->sb->sb_num_inodes, root_entries[i].ino);
            root_entries[i].ino = VSFS_INO_MAX;
            root_ino->i_nlink -= 1;
            return 0;
        }
    }
	return 0; // Shouldn't get here since path exists by assumption
}


/**
 * Change the modification time of a file or directory.
 *
 * Implements the utimensat() system call. See "man 2 utimensat" for details.
 *
 * NOTE: You only need to implement the setting of modification time (mtime).
 *       Timestamp modifications are not recursive.
 *
 * Assumptions (already verified by FUSE using getattr() calls):
 *   "path" exists.
 *
 * Errors: none
 *
 * @param path   path to the file or directory.
 * @param times  timestamps array. See "man 2 utimensat" for details.
 * @return       0 on success; -errno on failure.
 */
static int vsfs_utimens(const char *path, const struct timespec times[2])
{
	fs_ctx *fs = get_fs();

	// path with either the time passed as argument or the current time,
	// according to the utimensat man page

	// 0. Check if there is actually anything to be done.
	if (times[1].tv_nsec == UTIME_OMIT) {
		// Nothing to do.
		return 0;
	}

	// 1. Find the inode for the final component in path
    vsfs_ino_t ino;
    int ret = path_lookup(path, &ino);
    if (ret != 0) {
        return ret;
    }
    vsfs_inode *inode = &fs->itable[ino];

	// 2. Update the mtime for that inode.
	//    This code is commented out to avoid failure until you have set
	//    'ino' to point to the inode structure for the inode to update.
	if (times[1].tv_nsec == UTIME_NOW) {
		if (clock_gettime(CLOCK_REALTIME, &(inode->i_mtime)) != 0) {
            // clock_gettime should not fail, unless you give it a bad pointer to a timespec.
			assert(false);
		}
	} else {
		inode->i_mtime = times[1];
	}

	return 0;
}

/**
 * Change the size of a file.
 *
 * Implements the truncate() system call. Supports both extending and shrinking.
 * If the file is extended, the new uninitialized range at the end must be
 * filled with zeros.
 *
 * Assumptions (already verified by FUSE using getattr() calls):
 *   "path" exists and is a file.
 *
 * Errors:
 *   ENOMEM  not enough memory (e.g. a malloc() call failed).
 *   ENOSPC  not enough free space in the file system.
 *   EFBIG   write would exceed the maximum file size.
 *
 * @param path  path to the file to set the size.
 * @param size  new file size in bytes.
 * @return      0 on success; -errno on error.
 */
static int vsfs_truncate(const char *path, off_t size)
{
	fs_ctx *fs = get_fs();
    vsfs_ino_t ino;
    int ret = path_lookup(path, &ino);
    if (ret) { // Path lookup did not succeed
        return ret; // Return the respective error code
    }

    vsfs_inode *inode = &fs->itable[ino];

    // Calculate number of blocks before and after truncate
    unsigned int new_blocks = div_round_up(size, VSFS_BLOCK_SIZE);
    unsigned int cur_blocks = div_round_up(inode->i_size, VSFS_BLOCK_SIZE);

    if (new_blocks > cur_blocks) {

        if (new_blocks > VSFS_NUM_DIRECT + VSFS_BLOCK_SIZE/sizeof(vsfs_blk_t)) {
            return -EFBIG; // Need more blocks than maximum amount an inode can have
        }

        if (new_blocks - cur_blocks > fs->sb->sb_free_blocks){
            return -ENOSPC; // Not enough free blocks in fs
        }

        // Need to add blocks
        for (unsigned int i = cur_blocks; i < new_blocks; i++) {

            unsigned int blk;
            bitmap_alloc(fs->dbmap, fs->sb->sb_num_blocks, &blk);

            if (i >= VSFS_NUM_DIRECT) {
                if (inode->i_indirect < fs->sb->sb_data_region || inode->i_indirect >= VSFS_BLK_MAX){
                    bitmap_alloc(fs->dbmap, fs->sb->sb_num_blocks, &inode->i_indirect);
                    // DO NOT COUNT INDIRECT in i_block
                    vsfs_blk_t *indirect_entries = (vsfs_blk_t *)(fs->image + inode->i_indirect * VSFS_BLOCK_SIZE);
                    indirect_entries[i - VSFS_NUM_DIRECT] = blk;

                }
            } else {
                inode->i_direct[i] = blk;
            }
            inode->i_blocks += 1;
            // zero out the new block
            memset((char *)(fs->image + blk * VSFS_BLOCK_SIZE), 0, VSFS_BLOCK_SIZE);
            fs->sb->sb_free_blocks -= 1;
        }
    } else if (new_blocks < cur_blocks) {
        // Need to remove blocks
        for (unsigned int i = new_blocks; i < cur_blocks; i++) {
            bitmap_free(fs->dbmap, fs->sb->sb_num_blocks, inode->i_direct[i]);
            inode->i_direct[i] = VSFS_BLK_UNASSIGNED;
            inode->i_blocks -= 1;
            fs->sb->sb_free_blocks += 1;
        }
    }

    inode->i_size = size;

    return 0;
}


/**
 * Read data from a file.
 *
 * Implements the pread() system call. Must return exactly the number of bytes
 * requested except on EOF (end of file). Reads from file ranges that have not
 * been written to must return ranges filled with zeros. You can assume that the
 * byte range from offset to offset + size is contained within a single block.
 *
 * Assumptions (already verified by FUSE using getattr() calls):
 *   "path" exists and is a file.
 *
 * Errors: none
 *
 * @param path    path to the file to read from.
 * @param buf     pointer to the buffer that receives the data.
 * @param size    buffer size (number of bytes requested).
 * @param offset  offset from the beginning of the file to read from.
 * @param fi      unused.
 * @return        number of bytes read on success; 0 if offset is beyond EOF;
 *                -errno on error.
 */
static int vsfs_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi)
{
	(void)fi;// unused
    fs_ctx *fs = get_fs();
    vsfs_ino_t ino;
    int ret = path_lookup(path, &ino);
    if (ret != 0) {
        return ret;
    }
    vsfs_inode *inode = &fs->itable[ino];

    if ((long unsigned int)offset >= inode->i_size) {
        return 0; // offset beyond eof
    }

    if (offset + size > inode->i_size) { // shouldn't happen by assumption but just to be safe
        size = inode->i_size - offset; // only read until end of block
    }

    int block_index = offset / VSFS_BLOCK_SIZE; // index of block to read from
    int block_offset = offset % VSFS_BLOCK_SIZE; // offset within block to start the read
    if (inode->i_direct[block_index] == VSFS_BLK_UNASSIGNED) {
        memset(buf, 0, size);
    } else {
        // read the data
        const char *block = (const char *)(fs->image + inode->i_direct[block_index] * VSFS_BLOCK_SIZE);
        memcpy(buf, block + block_offset, size);
    }

	return size;
}

/**
 * Write data to a file.
 *
 * Implements the pwrite() system call. Must return exactly the number of bytes
 * requested except on error. If the offset is beyond EOF (end of file), the
 * file must be extended. If the write creates a "hole" of uninitialized data,
 * the new uninitialized range must filled with zeros. You can assume that the
 * byte range from offset to offset + size is contained within a single block.
 *
 * Assumptions (already verified by FUSE using getattr() calls):
 *   "path" exists and is a file.
 *
 * Errors:
 *   ENOMEM  not enough memory (e.g. a malloc() call failed).
 *   ENOSPC  not enough free space in the file system.
 *   EFBIG   write would exceed the maximum file size 
 *
 * @param path    path to the file to write to.
 * @param buf     pointer to the buffer containing the data.
 * @param size    buffer size (number of bytes requested).
 * @param offset  offset from the beginning of the file to write to.
 * @param fi      unused.
 * @return        number of bytes written on success; -errno on error.
 */
static int vsfs_write(const char *path, const char *buf, size_t size,
                      off_t offset, struct fuse_file_info *fi)
{
	(void)fi;// unused
	fs_ctx *fs = get_fs();
    vsfs_ino_t ino;
    int ret = path_lookup(path, &ino);
    if (ret != 0) {
        return ret;
    }
    vsfs_inode *inode = &fs->itable[ino];

    int block_index = offset / VSFS_BLOCK_SIZE;
    int block_offset = offset % VSFS_BLOCK_SIZE;
    if (block_index >= VSFS_NUM_DIRECT) {
        return -ENOSPC; // to do use indirect
    }

    // Extend the file if offset is beyond current size
    if (offset + size > inode->i_size) {
        int ret = vsfs_truncate(path, offset + size);
        // truncate takes care of zeroing out new blocks
        if (ret != 0) {
            return ret;
        }
    }
    char *block = (char *)(fs->image + inode->i_direct[block_index] * VSFS_BLOCK_SIZE);
    memcpy(block + block_offset, buf, size);
	return size;
}


static struct fuse_operations vsfs_ops = {
	.destroy  = vsfs_destroy,
	.statfs   = vsfs_statfs,
	.getattr  = vsfs_getattr,
	.readdir  = vsfs_readdir,
	.create   = vsfs_create,
	.unlink   = vsfs_unlink,
	.utimens  = vsfs_utimens,
	.truncate = vsfs_truncate,
	.read     = vsfs_read,
	.write    = vsfs_write,
};

int main(int argc, char *argv[])
{
	vsfs_opts opts = {0};// defaults are all 0
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	if (!vsfs_opt_parse(&args, &opts)) return 1;

	fs_ctx fs = {0};
	if (!vsfs_init(&fs, &opts)) {
		fprintf(stderr, "Failed to mount the file system\n");
		return 1;
	}

	return fuse_main(args.argc, args.argv, &vsfs_ops, &fs);
}
