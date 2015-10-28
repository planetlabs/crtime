/*
 * Derived from debugfs.c in e2fsprogs and inspired by:
 * http://unix.stackexchange.com/questions/50177/birth-is-empty-on-ext4/50184
 *
 * Copyright (C) 1993 Theodore Ts'o.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 * Modifications by Robert Sanders <gt8134b@prism.gatech.edu>
 *
 * To build:
 * apt-get install e2fslibs-dev # or similar
 * gcc -o crtime crtime.c -lext2fs
 *
 * To let non-root users get creation times of files:
 * chown root ./crtime
 * chmod u+s ./crtime
 *
 * To run:
 * target=/path/to/some/file
 * inode=$(ls -di "${target}" | cut -d ' ' -f 1);
 * fs=$(df "${target}"  | tail -1 | awk '{print $1}');
 * ./crtime ${fs} ${inode}
 */
#include <ext2fs/ext2fs.h>
#include <stdio.h>

ext2_filsys current_fs;

static int open_filesystem(char *device)
{
    int retval;
    io_manager io_ptr = unix_io_manager;

    retval = ext2fs_open(device, 0, 0, 0, io_ptr, &current_fs);
    if (retval) {
        fprintf(stderr, "Error while opening filesystem: %d\n", retval);
        return retval;
    }
    return 0;
}

int debugfs_read_inode_full(ext2_ino_t ino, struct ext2_inode * inode, int bufsize)
{
    int retval;

    return ext2fs_read_inode_full(current_fs, ino, inode, bufsize);
}

int main(int argc, char *argv[]) {

    ext2_ino_t    inode;
    struct ext2_inode *inode_buf;
    struct ext2_inode_large *large_inode;
    int retval;

    if (argc != 3) {
        fprintf(stderr, "Usage: crtime <filesystem> <inode>\n");
        return -1;
    }

    retval = sscanf(argv[2], "%d", &inode);
    if (retval != 1) {
        fprintf(stderr, "Could not parse an integer from %s\n", argv[2]);
        return -1;
    }

    retval = open_filesystem(argv[1]);
    if (retval) {
        return retval;
    }

    inode_buf = (struct ext2_inode *)malloc(EXT2_INODE_SIZE(current_fs->super));
    if (!inode_buf) {
        fprintf(stderr, "Failed to allocate memory\n");
        return -1;
    }

    retval = ext2fs_read_inode_full(current_fs, inode, inode_buf, EXT2_INODE_SIZE(current_fs->super));
    if (retval) {
        fprintf(stderr, "Failed to read inode\n");
        free(inode_buf);
        return retval;
    }

    if (EXT2_INODE_SIZE(current_fs->super) <= EXT2_GOOD_OLD_INODE_SIZE) {
        fprintf(stderr, "Create time unavailable");
        return -1;
    }

    large_inode = (struct ext2_inode_large *)inode_buf;

    if (large_inode->i_extra_isize < 24) {
        fprintf(stderr, "Create time unavailable");
        return -1;
    }

    printf("%d\n", large_inode->i_crtime);

    free(inode_buf);
    ext2fs_close(current_fs);
    return 0;
}
