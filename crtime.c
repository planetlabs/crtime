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
 * fs=$(df "${target}"  | tail -1 | awk '{print $1}');
 * ./crtime ${fs} ${target}
 */
#include <ext2fs/ext2fs.h>
#include <stdio.h>
#include <sys/stat.h>

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

static int get_inode(char *f, ext2_ino_t *inode) {

    struct stat sb;

    if (stat(f, &sb) == -1) {
        perror("stat");
        return errno;
    }

    if ((sizeof(sb.st_ino) > sizeof(ext2_ino_t)) &&
        (sb.st_ino > 0xffffffff)) {
        fprintf(stderr,
                "Inode of %s is bigger than what e2fslibs supports\n",
                f);
        return -1;
    }
    memcpy((void *)inode, (void *)&sb.st_ino, sizeof(ext2_ino_t));
    return 0;
}

int main(int argc, char *argv[]) {

    ext2_ino_t inode;
    struct ext2_inode *inode_buf;
    struct ext2_inode_large *large_inode;
    int retval;

    if (argc != 3) {
        fprintf(stderr, "Usage: crtime <filesystem> <file>\n");
        return -1;
    }

    retval = get_inode(argv[2], &inode);
    if (retval) {
        return retval;
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
